#include "software_update_manager.h"
#include "linitunes_device.h"
#include "plist_helpers.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUrl>
#include <plist/plist.h>
#include <algorithm>
#include <cstdlib>

namespace {
constexpr auto kCatalogUrl = "https://itunes.apple.com/WebObjects/MZStore.woa/wa/com.apple.jingle.appserver.client.MZITunesClientCheck/version";

plist_t dictItem(plist_t node, const char *key)
{
    return node && plist_get_node_type(node) == PLIST_DICT ? plist_dict_get_item(node, key) : nullptr;
}

bool isDict(plist_t node)
{
    return node && plist_get_node_type(node) == PLIST_DICT;
}

QList<QPair<QString, plist_t>> dictItems(plist_t dict)
{
    QList<QPair<QString, plist_t>> items;
    if (!isDict(dict))
        return items;

    plist_dict_iter iter = nullptr;
    plist_dict_new_iter(dict, &iter);
    char *key = nullptr;
    plist_t value = nullptr;
    while (true) {
        plist_dict_next_item(dict, iter, &key, &value);
        if (!key)
            break;
        items.append({QString::fromUtf8(key), value});
        free(key);
        key = nullptr;
    }
    free(iter);
    return items;
}

void collectRestoreEntries(plist_t node, QVector<IpswEntry> &entries, const QString &productType)
{
    if (!isDict(node))
        return;

    const QString url = plist_helpers::stringVal(node, "FirmwareURL");
    if (!url.isEmpty()) {
        IpswEntry entry;
        entry.productType = productType;
        entry.version = plist_helpers::stringVal(node, "ProductVersion");
        entry.build = plist_helpers::stringVal(node, "BuildVersion");
        entry.url = url;
        entry.sha1 = plist_helpers::stringVal(node, "FirmwareSHA1");
        entries.append(entry);
        return;
    }

    for (const auto &item : dictItems(node))
        collectRestoreEntries(item.second, entries, productType);
}

plist_t resolveSameAs(plist_t deviceDict, plist_t buildDict, int depth = 0)
{
    if (!isDict(deviceDict) || !isDict(buildDict) || depth > 8)
        return buildDict;

    const QString sameAs = plist_helpers::stringVal(buildDict, "SameAs");
    if (sameAs.isEmpty())
        return buildDict;

    plist_t target = plist_dict_get_item(deviceDict, sameAs.toUtf8().constData());
    return target ? resolveSameAs(deviceDict, target, depth + 1) : buildDict;
}

QVector<int> versionParts(const QString &version)
{
    QVector<int> parts;
    static const QRegularExpression numberRe(QStringLiteral("(\\d+)"));
    auto it = numberRe.globalMatch(version);
    while (it.hasNext())
        parts.append(it.next().captured(1).toInt());
    return parts;
}

int compareVersions(const QString &a, const QString &b)
{
    const QVector<int> av = versionParts(a);
    const QVector<int> bv = versionParts(b);
    const int count = std::max(av.size(), bv.size());
    for (int i = 0; i < count; ++i) {
        const int ai = i < av.size() ? av[i] : 0;
        const int bi = i < bv.size() ? bv[i] : 0;
        if (ai != bi)
            return ai < bi ? -1 : 1;
    }
    return 0;
}

QString safeFilePart(QString value)
{
    value.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_.-]")), QStringLiteral("_"));
    return value;
}

QString headerValue(QNetworkReply *reply, const QByteArray &name)
{
    for (const auto &header : reply->rawHeaderPairs()) {
        if (header.first.compare(name, Qt::CaseInsensitive) == 0)
            return QString::fromLatin1(header.second).trimmed();
    }
    return {};
}

qint64 totalFromContentRange(const QString &range)
{
    const int slash = range.lastIndexOf('/');
    if (slash < 0)
        return 0;
    bool ok = false;
    const qint64 total = range.mid(slash + 1).toLongLong(&ok);
    return ok ? total : 0;
}

bool isMultipartEtag(const QString &etag)
{
    return etag.contains(QRegularExpression(QStringLiteral("-[0-9]+\"?$")));
}
}

QVariantMap IpswEntry::toMap() const
{
    QVariantMap map;
    map["product_type"] = productType;
    map["version"] = version;
    map["build"] = build;
    map["url"] = url;
    map["sha1"] = sha1;
    map["sha256"] = sha256;
    map["md5"] = md5;
    map["size"] = size;
    map["size_text"] = iDevice::format_bytes(static_cast<uint64_t>(std::max<qint64>(0, size)));
    map["signed"] = signedRestore;
    return map;
}

QString IpswEntry::key() const
{
    return productType + QStringLiteral("|") + version + QStringLiteral("|") + build + QStringLiteral("|") + url;
}

SoftwareUpdateManager::SoftwareUpdateManager(QObject *parent)
    : QObject(parent),
      m_sha256(QCryptographicHash::Sha256),
      m_sha1(QCryptographicHash::Sha1)
{
    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(30000);
    connect(&m_idleTimer, &QTimer::timeout, this, [this]() {
        if (m_downloading && !m_cancelled)
            retryDownload(QStringLiteral("Download stalled."));
    });
}

QVariantList SoftwareUpdateManager::updateCandidates() const
{
    return toVariantList(m_updateCandidates);
}

QVariantList SoftwareUpdateManager::restoreCandidates() const
{
    return toVariantList(m_restoreCandidates);
}

void SoftwareUpdateManager::check(const QString &productType, const QString &currentVersion, const QString &currentBuild,
                                  bool silent)
{
    if (m_busy || productType.isEmpty())
        return;

    const int generation = ++m_generation;
    m_busy = true;
    m_silentCheck = silent;
    m_currentVersion = currentVersion;
    m_currentBuild = currentBuild;
    if (silent)
        emit changed();
    else
        setStatus(QStringLiteral("Checking for updates…"));

    auto *reply = m_net.get(requestFor(QString::fromLatin1(kCatalogUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, productType, generation]() {
        reply->deleteLater();
        if (generation != m_generation) {
            m_busy = false;
            m_silentCheck = false;
            m_metadataQueue.clear();
            emit changed();
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            m_busy = false;
            if (m_silentCheck) {
                m_silentCheck = false;
                m_status = QStringLiteral("idle");
                m_error.clear();
                emit changed();
            } else {
                setStatus(QStringLiteral("Check failed"), reply->errorString());
            }
            return;
        }
        finishCheck(parseCatalog(reply->readAll(), productType));
    });
}

void SoftwareUpdateManager::downloadUpdate(int index)
{
    downloadFromList(m_updateCandidates, index);
}

void SoftwareUpdateManager::downloadRestore(int index)
{
    downloadFromList(m_restoreCandidates, index);
}

void SoftwareUpdateManager::cancelDownload()
{
    if (!m_downloading)
        return;

    m_cancelled = true;
    stopDownloadReply();
    m_idleTimer.stop();
    if (m_downloadFile.isOpen())
        m_downloadFile.close();
    QFile::remove(partPath(m_downloadEntry));
    m_downloading = false;
    m_downloadProgress = 0;
    setStatus(QStringLiteral("Download cancelled."));
}

void SoftwareUpdateManager::reset()
{
    ++m_generation;
    m_idleTimer.stop();
    stopDownloadReply();
    if (m_downloadFile.isOpen())
        m_downloadFile.close();
    if (!m_downloadEntry.url.isEmpty())
        QFile::remove(partPath(m_downloadEntry));

    m_updateCandidates.clear();
    m_restoreCandidates.clear();
    m_metadataQueue.clear();
    m_currentVersion.clear();
    m_currentBuild.clear();
    m_status = QStringLiteral("idle");
    m_error.clear();
    m_downloadedPath.clear();
    m_busy = false;
    m_downloading = false;
    m_cancelled = false;
    m_downloadProgress = 0;
    m_downloadedBytes = 0;
    m_downloadEntry = IpswEntry();
    m_downloadRetries = 0;
    m_silentCheck = false;
    emit changed();
}

void SoftwareUpdateManager::setStatus(const QString &status, const QString &error)
{
    m_status = status;
    m_error = error;
    emit changed();
}

void SoftwareUpdateManager::finishCheck(QVector<IpswEntry> entries)
{
    std::sort(entries.begin(), entries.end(), [](const IpswEntry &a, const IpswEntry &b) {
        const int versionCompare = compareVersions(a.version, b.version);
        if (versionCompare != 0)
            return versionCompare > 0;
        const int buildCompare = compareVersions(a.build, b.build);
        return buildCompare == 0 ? a.build > b.build : buildCompare > 0;
    });

    QVector<IpswEntry> restoreCandidates;
    QVector<IpswEntry> updateCandidates;
    QSet<QString> seen;
    for (const IpswEntry &entry : entries) {
        if (entry.url.isEmpty() || seen.contains(entry.key()) || !isSafeAppleUrl(entry.url))
            continue;
        seen.insert(entry.key());
        restoreCandidates.append(entry);
        if (compareVersions(entry.version, m_currentVersion) > 0 ||
            (entry.version == m_currentVersion && !m_currentBuild.isEmpty() && compareVersions(entry.build, m_currentBuild) > 0)) {
            updateCandidates.append(entry);
        }
    }

    m_restoreCandidates = restoreCandidates;
    m_updateCandidates = updateCandidates;
    m_metadataQueue = m_restoreCandidates;
    fetchNextMetadata();
}

void SoftwareUpdateManager::fetchNextMetadata()
{
    if (m_metadataQueue.isEmpty()) {
        m_busy = false;
        m_downloadedPath.clear();
        for (const IpswEntry &candidate : std::as_const(m_updateCandidates)) {
            m_downloadedPath = verifiedPathFor(candidate);
            if (!m_downloadedPath.isEmpty())
                break;
        }

        const QString status = m_updateCandidates.isEmpty()
            ? QStringLiteral("Your device is up to date.")
            : QStringLiteral("Update available: %1").arg(m_updateCandidates.first().version);
        m_silentCheck = false;
        setStatus(status);
        return;
    }

    const IpswEntry entry = m_metadataQueue.takeFirst();
    const int generation = m_generation;
    auto *reply = m_net.head(requestFor(entry.url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, entry, generation]() mutable {
        reply->deleteLater();
        if (generation != m_generation) {
            m_busy = false;
            m_silentCheck = false;
            m_metadataQueue.clear();
            emit changed();
            return;
        }
        IpswEntry updated = entry;
        if (reply->error() == QNetworkReply::NoError)
            applyMetadata(reply, &updated);
        if (updated.size <= 0 || (updated.sha256.isEmpty() && updated.sha1.isEmpty())) {
            startRangeMetadata(updated);
            return;
        }

        applyEntryToCandidates(updated);
        fetchNextMetadata();
    });
}

void SoftwareUpdateManager::applyMetadata(QNetworkReply *reply, IpswEntry *entry)
{
    entry->sha256 = headerValue(reply, "x-amz-meta-digest-sha256");
    const QString cdnSha1 = headerValue(reply, "x-amz-meta-digest-sh1");
    if (!cdnSha1.isEmpty())
        entry->sha1 = cdnSha1;

    bool ok = false;
    const qint64 contentLength = headerValue(reply, "Content-Length").toLongLong(&ok);
    if (ok && contentLength > 0)
        entry->size = contentLength;

    const QString etag = headerValue(reply, "ETag").remove('"');
    if (!isMultipartEtag(etag) && etag.size() == 32)
        entry->md5 = etag;
}

void SoftwareUpdateManager::applyEntryToCandidates(const IpswEntry &entry)
{
    auto apply = [&entry](QVector<IpswEntry> &entries) {
        for (IpswEntry &candidate : entries) {
            if (candidate.key() == entry.key())
                candidate = entry;
        }
    };
    apply(m_restoreCandidates);
    apply(m_updateCandidates);
}

void SoftwareUpdateManager::startRangeMetadata(const IpswEntry &entry)
{
    QNetworkRequest request = requestFor(entry.url);
    request.setRawHeader("Range", "bytes=0-0");
    const int generation = m_generation;
    auto *reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, entry, generation]() mutable {
        reply->deleteLater();
        if (generation != m_generation) {
            m_busy = false;
            m_silentCheck = false;
            m_metadataQueue.clear();
            emit changed();
            return;
        }
        IpswEntry updated = entry;
        if (reply->error() == QNetworkReply::NoError) {
            applyMetadata(reply, &updated);
            const qint64 total = totalFromContentRange(headerValue(reply, "Content-Range"));
            if (total > 0)
                updated.size = total;
        }

        applyEntryToCandidates(updated);
        fetchNextMetadata();
    });
}

void SoftwareUpdateManager::downloadFromList(const QVector<IpswEntry> &entries, int index)
{
    if (index >= 0 && index < entries.size()) {
        m_downloadRetries = 0;
        startDownload(entries.at(index));
    }
}

void SoftwareUpdateManager::startDownload(const IpswEntry &entry, bool retry)
{
    if ((m_downloading && !retry) || entry.url.isEmpty() || !isSafeAppleUrl(entry.url))
        return;

    const QString existing = verifiedPathFor(entry);
    if (!existing.isEmpty()) {
        m_downloading = false;
        m_downloadProgress = 100;
        m_downloadedPath = existing;
        setStatus(QStringLiteral("IPSW already downloaded and verified."));
        return;
    }

    const int generation = ++m_generation;
    QDir().mkpath(cacheDir());
    m_downloadEntry = entry;
    m_downloadedPath.clear();
    m_downloadProgress = 0;
    m_downloadedBytes = 0;
    m_cancelled = false;
    m_sha256.reset();
    m_sha1.reset();

    const QString part = partPath(entry);
    QFile::remove(part);

    if (entry.size > 0) {
        const QStorageInfo storage(QFileInfo(part).absolutePath());
        if (storage.isValid() && storage.bytesAvailable() < entry.size) {
            m_downloading = false;
            setStatus(QStringLiteral("Download failed"),
                      QStringLiteral("Not enough free disk space to download this IPSW. Free up at least %1 or choose a drive with more space.")
                          .arg(iDevice::format_bytes(static_cast<uint64_t>(entry.size))));
            return;
        }
    }

    m_downloadFile.setFileName(part);
    if (!m_downloadFile.open(QIODevice::WriteOnly)) {
        m_downloading = false;
        setStatus(QStringLiteral("Download failed"), QStringLiteral("Could not create the temporary IPSW file."));
        return;
    }

    m_downloading = true;
    setStatus(retry
              ? QStringLiteral("Retrying download %1/%2…").arg(m_downloadRetries).arg(3)
              : QStringLiteral("Downloading %1…").arg(entry.version));
    m_downloadReply = m_net.get(requestFor(entry.url));
    restartIdleTimer();

    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, [this, generation](qint64 received, qint64 total) {
        if (generation != m_generation)
            return;
        restartIdleTimer();
        if (total > 0)
            m_downloadProgress = qBound(0.0, (received * 100.0) / total, 100.0);
        emit changed();
    });
    connect(m_downloadReply, &QNetworkReply::readyRead, this, [this, generation]() {
        if (generation != m_generation || !m_downloadReply || !m_downloadReply->isOpen())
            return;
        restartIdleTimer();
        const QByteArray data = m_downloadReply->readAll();
        if (data.isEmpty())
            return;
        if (!writeDownloadChunk(data)) {
            stopDownloadReply();
            if (m_downloadFile.isOpen())
                m_downloadFile.close();
            failDownload(QStringLiteral("Could not write the IPSW to disk. The disk may be full."));
            return;
        }
    });
    connect(m_downloadReply, &QNetworkReply::finished, this, [this, generation]() {
        if (generation == m_generation)
            finishDownload(m_cancelled);
    });
}

void SoftwareUpdateManager::finishDownload(bool cancelled)
{
    m_idleTimer.stop();
    auto *reply = m_downloadReply.data();
    const bool networkError = reply && reply->error() != QNetworkReply::NoError;
    const QString networkErrorString = networkError ? reply->errorString() : QString();
    if (reply && !cancelled && !networkError && reply->isOpen()) {
        const QByteArray tail = reply->readAll();
        if (!tail.isEmpty() && !writeDownloadChunk(tail)) {
            reply->deleteLater();
            m_downloadReply.clear();
            m_downloadFile.close();
            failDownload(QStringLiteral("Could not write the IPSW to disk. The disk may be full."));
            return;
        }
    }
    if (reply)
        reply->deleteLater();
    m_downloadReply.clear();
    m_downloadFile.close();

    if (cancelled) {
        QFile::remove(partPath(m_downloadEntry));
        m_downloading = false;
        m_downloadProgress = 0;
        setStatus(QStringLiteral("Download cancelled."));
        return;
    }

    if (networkError) {
        retryDownload(networkErrorString);
        return;
    }
    if (m_downloadEntry.size > 0 && m_downloadedBytes != m_downloadEntry.size) {
        failDownload(QStringLiteral("Downloaded size does not match Apple metadata."));
        return;
    }
    if (!verifyHash(m_downloadEntry, m_sha256.result(), m_sha1.result())) {
        failDownload(QStringLiteral("Downloaded IPSW failed hash verification."));
        return;
    }

    const QString final = finalPath(m_downloadEntry);
    QFile::remove(final);
    if (!QFile::rename(partPath(m_downloadEntry), final)) {
        failDownload(QStringLiteral("Could not finalize verified IPSW."));
        return;
    }

    m_downloadedPath = final;
    m_downloading = false;
    m_downloadProgress = 100;
    rememberVerified(m_downloadEntry, final);
    setStatus(QStringLiteral("IPSW downloaded and verified."));
}

void SoftwareUpdateManager::retryDownload(const QString &message)
{
    m_idleTimer.stop();
    stopDownloadReply();
    if (m_downloadFile.isOpen())
        m_downloadFile.close();
    QFile::remove(partPath(m_downloadEntry));

    if (m_cancelled || m_downloadRetries >= 3) {
        failDownload(message);
        return;
    }

    ++m_downloadRetries;
    m_downloadProgress = 0;
    const int generation = m_generation;
    const IpswEntry entry = m_downloadEntry;
    QTimer::singleShot(2000, this, [this, entry, generation]() {
        if (generation != m_generation || m_cancelled || !m_downloading || m_downloadEntry.key() != entry.key())
            return;
        startDownload(entry, true);
    });
    setStatus(QStringLiteral("Connection interrupted. Retrying…"));
}

void SoftwareUpdateManager::stopDownloadReply()
{
    if (!m_downloadReply)
        return;
    disconnect(m_downloadReply, nullptr, this, nullptr);
    m_downloadReply->abort();
    m_downloadReply->deleteLater();
    m_downloadReply.clear();
}

void SoftwareUpdateManager::failDownload(const QString &message)
{
    m_idleTimer.stop();
    QFile::remove(partPath(m_downloadEntry));
    m_downloading = false;
    m_downloadProgress = 0;
    setStatus(QStringLiteral("Download failed"), message);
}

bool SoftwareUpdateManager::writeDownloadChunk(const QByteArray &data)
{
    if (m_downloadFile.write(data) != data.size())
        return false;

    m_downloadedBytes += data.size();
    m_sha256.addData(data);
    m_sha1.addData(data);
    return true;
}

void SoftwareUpdateManager::restartIdleTimer()
{
    if (m_downloading && !m_cancelled)
        m_idleTimer.start();
}

QVector<IpswEntry> SoftwareUpdateManager::parseCatalog(const QByteArray &data, const QString &productType) const
{
    QVector<IpswEntry> entries;
    plist_t root = nullptr;
    plist_format_t format = PLIST_FORMAT_NONE;
    if (plist_from_memory(data.constData(), static_cast<uint32_t>(data.size()), &root, &format) != PLIST_ERR_SUCCESS || !root)
        return entries;

    plist_t versions = dictItem(root, "MobileDeviceSoftwareVersionsByVersion");
    for (const auto &versionGroup : dictItems(versions)) {
        plist_t devices = dictItem(versionGroup.second, "MobileDeviceSoftwareVersions");
        plist_t device = plist_dict_get_item(devices, productType.toUtf8().constData());
        if (!device)
            continue;
        for (const auto &build : dictItems(device)) {
            plist_t resolved = resolveSameAs(device, build.second);
            collectRestoreEntries(resolved, entries, productType);
        }
    }

    plist_free(root);
    return entries;
}

QVariantList SoftwareUpdateManager::toVariantList(const QVector<IpswEntry> &entries) const
{
    QVariantList list;
    for (const IpswEntry &entry : entries)
        list.append(entry.toMap());
    return list;
}

QString SoftwareUpdateManager::cacheDir() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return QDir(base).filePath(QStringLiteral("ipsw"));
}

QString SoftwareUpdateManager::indexPath() const
{
    return QDir(cacheDir()).filePath(QStringLiteral("index.json"));
}

QString SoftwareUpdateManager::finalPath(const IpswEntry &entry) const
{
    const QString name = QStringLiteral("%1_%2_%3.ipsw")
        .arg(safeFilePart(entry.productType), safeFilePart(entry.version), safeFilePart(entry.build));
    return QDir(cacheDir()).filePath(name);
}

QString SoftwareUpdateManager::partPath(const IpswEntry &entry) const
{
    return finalPath(entry) + QStringLiteral(".part");
}

QJsonArray SoftwareUpdateManager::readIndex() const
{
    QFile file(indexPath());
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return QJsonDocument::fromJson(file.readAll()).array();
}

void SoftwareUpdateManager::writeIndex(const QJsonArray &items) const
{
    QDir().mkpath(cacheDir());
    QSaveFile file(indexPath());
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument(items).toJson(QJsonDocument::Indented));
    file.commit();
}

QString SoftwareUpdateManager::verifiedPathFor(const IpswEntry &entry) const
{
    const QJsonArray items = readIndex();
    for (const QJsonValue &value : items) {
        const QJsonObject obj = value.toObject();
        const QString path = obj.value(QStringLiteral("path")).toString();
        if (obj.value(QStringLiteral("product_type")).toString() == entry.productType &&
            obj.value(QStringLiteral("build")).toString() == entry.build &&
            obj.value(QStringLiteral("url")).toString() == entry.url &&
            QFileInfo::exists(path)) {
            return path;
        }
    }
    return {};
}

void SoftwareUpdateManager::rememberVerified(const IpswEntry &entry, const QString &path)
{
    QJsonArray items = readIndex();
    QJsonArray kept;
    for (const QJsonValue &value : items) {
        const QJsonObject obj = value.toObject();
        if (!(obj.value(QStringLiteral("product_type")).toString() == entry.productType &&
              obj.value(QStringLiteral("build")).toString() == entry.build &&
              obj.value(QStringLiteral("url")).toString() == entry.url)) {
            kept.append(obj);
        }
    }

    QJsonObject obj;
    obj["product_type"] = entry.productType;
    obj["version"] = entry.version;
    obj["build"] = entry.build;
    obj["url"] = entry.url;
    obj["size"] = entry.size;
    obj["sha256"] = entry.sha256;
    obj["sha1"] = entry.sha1;
    obj["md5"] = entry.md5;
    obj["path"] = path;
    obj["verified_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    kept.append(obj);
    writeIndex(kept);
}

QNetworkRequest SoftwareUpdateManager::requestFor(const QString &url) const
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("LiniTunes/AppleCatalog"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return request;
}

bool SoftwareUpdateManager::isSafeAppleUrl(const QString &url) const
{
    const QUrl parsed(url);
    const QString host = parsed.host().toLower();
    return parsed.scheme() == QStringLiteral("https") &&
        (host == QStringLiteral("itunes.apple.com") ||
         host == QStringLiteral("updates.cdn-apple.com") ||
         host.endsWith(QStringLiteral(".apple.com")) ||
         host.endsWith(QStringLiteral(".cdn-apple.com")));
}

bool SoftwareUpdateManager::verifyHash(const IpswEntry &entry, const QByteArray &sha256, const QByteArray &sha1) const
{
    if (!entry.sha256.isEmpty())
        return QString::fromLatin1(sha256.toHex()).compare(entry.sha256, Qt::CaseInsensitive) == 0;
    if (!entry.sha1.isEmpty())
        return QString::fromLatin1(sha1.toHex()).compare(entry.sha1, Qt::CaseInsensitive) == 0;
    return false;
}

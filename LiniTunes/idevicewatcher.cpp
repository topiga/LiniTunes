#include "idevicewatcher.h"
#include "usbmuxd_helpers.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDesktopServices>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QUrl>
#include <plist/plist.h>
#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <utility>
#include <idevice++/usbmuxd.hpp>

static SoftwareUpdateManager *softwareManager(iDevice *device)
{
    return device ? device->softwareUpdateManager() : nullptr;
}

static QString muxAddressFromKey(const QString &key, uint32_t *deviceId)
{
    const int separator = key.lastIndexOf(QLatin1Char('#'));
    if (separator <= 0)
        return {};

    bool ok = false;
    const uint32_t parsedDeviceId = key.mid(separator + 1).toUInt(&ok);
    if (!ok)
        return {};

    if (deviceId)
        *deviceId = parsedDeviceId;

    return usbmuxd_helpers::muxAddressFromKeySource(key.left(separator));
}

constexpr int kDisconnectDebounceCycles = 3;

static void retryDelay(std::atomic<bool> &running, int slices = 20) {
    for (int i = 0; i < slices && running; ++i)
        QThread::msleep(100);
}

// ---- UsbmuxdListener ------------------------------------------------------

UsbmuxdListener::UsbmuxdListener(QObject *parent)
    : QObject(parent) {}

UsbmuxdListener::~UsbmuxdListener()
{
    m_running = false;
}

void UsbmuxdListener::stop()
{
    m_running = false;
}

void UsbmuxdListener::run()
{
    m_running = true;
    QSet<QString> known;
    QHash<QString, int> missingCounts;
    QSet<QString> loggedUnavailableNetmuxd;

    while (m_running) {
        QSet<QString> seen;

        for (const auto &source : usbmuxd_helpers::candidateMuxSources()) {
            auto connResult = usbmuxd_helpers::connect(source.address, 0);
            if (connResult.is_err()) {
                if (source.netmuxd && !loggedUnavailableNetmuxd.contains(source.key)) {
                    auto &err = connResult.unwrap_err();
                    qDebug("netmuxd unavailable at %s: %s",
                           qPrintable(source.displayName), err.message.c_str());
                    loggedUnavailableNetmuxd.insert(source.key);
                }
                continue;
            }

            if (source.netmuxd && loggedUnavailableNetmuxd.remove(source.key))
                qDebug("netmuxd available at %s", qPrintable(source.displayName));

            auto conn = std::move(connResult.unwrap());
            auto devicesResult = conn.get_devices();
            if (devicesResult.is_err())
                continue;

            for (const auto &device : devicesResult.unwrap()) {
                auto udid = device.get_udid();
                auto id = device.get_id();
                if (udid.is_none() || id.is_none())
                    continue;

                const uint32_t deviceId = id.unwrap();
                const QString key = usbmuxd_helpers::muxKey(source.address, deviceId);
                seen.insert(key);
                if (known.contains(key)) {
                    missingCounts.remove(key);
                    continue;
                }

                known.insert(key);
                bool networkConnection = false;
                auto type = device.get_connection_type();
                if (type.is_some())
                    networkConnection = type.unwrap() == IdeviceFFI::UsbmuxdConnectionType::Value::Network;

                const QString deviceUdid = QString::fromStdString(udid.unwrap());
                qDebug("Listener: device connected %s (id=%u, mux=%s, transport=%s)",
                       qPrintable(deviceUdid), deviceId,
                       qPrintable(source.displayName),
                       networkConnection ? "Network" : "USB");
                emit deviceConnected(deviceUdid, deviceId, source.address, networkConnection);
            }
        }

        QStringList disconnectedKeys;
        for (const QString &key : std::as_const(known)) {
            if (seen.contains(key)) {
                missingCounts.remove(key);
                continue;
            }

            const int missingCount = missingCounts.value(key) + 1;
            if (missingCount < kDisconnectDebounceCycles) {
                missingCounts.insert(key, missingCount);
                continue;
            }

            uint32_t deviceId = 0;
            const QString muxAddress = muxAddressFromKey(key, &deviceId);
            if (deviceId == 0)
                continue;
            qDebug("Listener: device disconnected (id=%u, mux=%s)",
                   deviceId, qPrintable(usbmuxd_helpers::muxDisplayName(muxAddress)));
            emit deviceDisconnected(muxAddress, deviceId);
            disconnectedKeys.append(key);
        }

        for (const QString &key : disconnectedKeys) {
            known.remove(key);
            missingCounts.remove(key);
        }
        retryDelay(m_running, 10);
    }
}

// ---- DeviceInitWorker -----------------------------------------------------

void DeviceInitWorker::doInit(const QString &udid, uint32_t deviceId, const QString &muxAddress,
                              bool networkConnection, bool enableWifiSync)
{
    auto *dev = new iDevice();
    auto addr = usbmuxd_helpers::makeAddr(muxAddress);

    if (dev->init(udid, deviceId, muxAddress, networkConnection, enableWifiSync, std::move(addr))) {
        emit initDone(dev);
    } else {
        qDebug("Worker: device init failed: %s", qPrintable(udid));
        delete dev;
        emit initFailed(udid);
    }
}

// ---- iDeviceWatcher -------------------------------------------------------

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{
    QSettings settings(QStringLiteral("LiniTunes"), QStringLiteral("LiniTunes"));
    m_backupFolder = settings.value(QStringLiteral("backup_folder")).toString();
    m_wifiSyncEnabled = settings.value(QStringLiteral("wifi_sync_enabled"), true).toBool();
    m_wifiSyncKnownUdids = settings.value(QStringLiteral("wifi_sync_known_udids")).toStringList();
    m_lastWifiSyncUdid = settings.value(QStringLiteral("wifi_sync_last_udid")).toString();

    m_listener = new UsbmuxdListener();
    m_listener->moveToThread(&m_listenerThread);
    connect(&m_listenerThread, &QThread::started, m_listener, &UsbmuxdListener::run);
    connect(m_listener, &UsbmuxdListener::deviceConnected,
            this, &iDeviceWatcher::onDeviceConnected);
    connect(m_listener, &UsbmuxdListener::deviceDisconnected,
            this, &iDeviceWatcher::onDeviceDisconnected);
    connect(&m_listenerThread, &QThread::finished,
            m_listener, &QObject::deleteLater);

    m_worker = new DeviceInitWorker();
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    connect(m_worker, &DeviceInitWorker::initDone,
            this, &iDeviceWatcher::onDeviceInitDone);
    connect(m_worker, &DeviceInitWorker::initFailed,
            this, &iDeviceWatcher::onDeviceInitFailed);
}

iDeviceWatcher::~iDeviceWatcher()
{
    m_listenerThread.quit();
    m_listenerThread.wait();
    m_workerThread.quit();
    m_workerThread.wait();

    for (auto *d : Devices) delete d;
    Devices.clear();
}

void iDeviceWatcher::start()
{
    m_listenerThread.start();
    m_workerThread.start();
}

void iDeviceWatcher::setBackupFolder(const QString &folder)
{
    if (m_backupFolder == folder)
        return;

    m_backupFolder = folder;
    QSettings settings(QStringLiteral("LiniTunes"), QStringLiteral("LiniTunes"));
    if (folder.isEmpty())
        settings.remove(QStringLiteral("backup_folder"));
    else
        settings.setValue(QStringLiteral("backup_folder"), folder);
    emit backupFolderChanged();
}

void iDeviceWatcher::setWifiSyncEnabled(bool enabled)
{
    if (m_wifiSyncEnabled == enabled)
        return;

    m_wifiSyncEnabled = enabled;
    QSettings(QStringLiteral("LiniTunes"), QStringLiteral("LiniTunes"))
        .setValue(QStringLiteral("wifi_sync_enabled"), enabled);
    emit wifiSyncEnabledChanged();
}

void iDeviceWatcher::connectDeviceSignals(iDevice *dev)
{
    dev->ensureSoftwareUpdateManager();
    connect(dev, &iDevice::storageSyncChanged,
            this, &iDeviceWatcher::storageSyncChanged);
    connect(dev, &iDevice::backupChanged,
            this, &iDeviceWatcher::backupChanged);
    connect(dev->softwareUpdateManager(), &SoftwareUpdateManager::changed,
            this, &iDeviceWatcher::softwareChanged);
}

void iDeviceWatcher::initEndpoint(const MuxEndpoint &endpoint)
{
    const bool enableWifiSync = m_wifiSyncEnabled;
    QMetaObject::invokeMethod(m_worker,
        [this, endpoint, enableWifiSync]() {
            m_worker->doInit(endpoint.udid, endpoint.deviceId,
                             endpoint.muxAddress, endpoint.networkConnection,
                             enableWifiSync);
        },
        Qt::QueuedConnection);
}

void iDeviceWatcher::onDeviceConnected(const QString &udid, uint32_t deviceId,
                                       const QString &muxAddress, bool networkConnection)
{
    const QString key = usbmuxd_helpers::muxKey(muxAddress, deviceId);
    MuxEndpoint endpoint{udid, deviceId, muxAddress, networkConnection};
    m_muxEndpoints.insert(key, endpoint);

    auto *existing = deviceForUdid(udid);
    if (shouldSwitchToEndpoint(existing, endpoint))
        initEndpoint(endpoint);
}

void iDeviceWatcher::onDeviceDisconnected(const QString &muxAddress, uint32_t deviceId)
{
    removeDeviceByMuxKey(usbmuxd_helpers::muxKey(muxAddress, deviceId));
}

void iDeviceWatcher::onDeviceInitDone(iDevice *dev)
{
    qDebug("Device: %s | %s | %s",
           qPrintable(dev->device_name()),
           qPrintable(dev->product_type()),
           qPrintable(dev->marketing_name()));

    connectDeviceSignals(dev);
    if (dev->wifiSyncAvailable())
        rememberWifiSyncDevice(dev->udid());
    dev->softwareUpdateManager()->check(dev->product_type(),
                                        dev->product_version(),
                                        dev->build_version(),
                                        true);

    for (int i = 0; i < Devices.size(); ++i) {
        if (Devices[i]->udid() != dev->udid())
            continue;
        if (!shouldUseInitializedDevice(Devices[i], dev)) {
            delete dev;
            return;
        }

        const bool wasCurrent = m_currentDevice == Devices[i];
        delete Devices[i];
        Devices[i] = dev;
        if (wasCurrent)
            m_currentDevice = dev;
        updateLists();
        return;
    }

    Devices.append(dev);
    updateLists();
}

void iDeviceWatcher::onDeviceInitFailed(const QString &udid)
{
    qDebug("Device init failed: %s", qPrintable(udid));
}

void iDeviceWatcher::rememberWifiSyncDevice(const QString &udid)
{
    if (udid.isEmpty())
        return;

    if (!m_wifiSyncKnownUdids.contains(udid))
        m_wifiSyncKnownUdids.append(udid);
    m_lastWifiSyncUdid = udid;

    QSettings settings(QStringLiteral("LiniTunes"), QStringLiteral("LiniTunes"));
    settings.setValue(QStringLiteral("wifi_sync_known_udids"), m_wifiSyncKnownUdids);
    settings.setValue(QStringLiteral("wifi_sync_last_udid"), m_lastWifiSyncUdid);
}

iDeviceWatcher::MuxEndpoint iDeviceWatcher::preferredEndpointForUdid(const QString &udid) const
{
    MuxEndpoint fallback;
    for (const MuxEndpoint &endpoint : std::as_const(m_muxEndpoints)) {
        if (endpoint.udid != udid)
            continue;
        if (fallback.udid.isEmpty() || (!endpoint.networkConnection && fallback.networkConnection))
            fallback = endpoint;
    }
    return fallback;
}

bool iDeviceWatcher::shouldSwitchToEndpoint(const iDevice *existing, const MuxEndpoint &candidate) const
{
    if (!existing)
        return true;
    const QString candidateKey = usbmuxd_helpers::muxKey(candidate.muxAddress, candidate.deviceId);
    if (existing->muxKey() == candidateKey)
        return false;
    return !candidate.networkConnection && existing->networkConnection();
}

bool iDeviceWatcher::shouldUseInitializedDevice(const iDevice *existing, const iDevice *candidate) const
{
    if (!existing)
        return true;
    if (existing->muxKey() == candidate->muxKey())
        return true;
    return !candidate->networkConnection() && existing->networkConnection();
}

iDevice *iDeviceWatcher::deviceForUdid(const QString &udid) const
{
    for (auto *device : Devices) {
        if (device->udid() == udid)
            return device;
    }
    return nullptr;
}

void iDeviceWatcher::removeDeviceByMuxKey(const QString &key)
{
    const MuxEndpoint removed = m_muxEndpoints.take(key);
    if (removed.udid.isEmpty())
        return;

    for (int i = 0; i < Devices.size(); ++i) {
        if (Devices[i]->udid() != removed.udid || Devices[i]->muxKey() != key)
            continue;

        if (m_currentDevice == Devices[i])
            m_currentDevice = nullptr;
        delete Devices[i];
        Devices.removeAt(i);
        Devices.squeeze();

        const MuxEndpoint fallback = preferredEndpointForUdid(removed.udid);
        if (!fallback.udid.isEmpty())
            initEndpoint(fallback);

        updateLists();
        return;
    }
}

void iDeviceWatcher::updateLists()
{
    m_udidList.clear();
    for (auto *d : Devices) {
        if (d->device_connected())
            m_udidList.append(d->udid());
    }

    if (Devices.isEmpty()) {
        m_currentDevice = nullptr;
        emit currentDeviceChanged();
        emit storageSyncChanged();
        emit backupChanged();
        emit softwareChanged();
    } else if (Devices.size() == 1) {
        switchCurrentDevice(Devices.at(0)->udid());
    } else if (m_currentDevice == nullptr) {
        switchCurrentDevice(Devices.at(0)->udid());
    }

    emit udidListChanged();
}

void iDeviceWatcher::switchCurrentDevice(const QString &udid)
{
    if (udid.isEmpty()) {
        m_currentDevice = nullptr;
        emit currentDeviceChanged();
        emit storageSyncChanged();
        emit backupChanged();
        emit softwareChanged();
        return;
    }

    for (auto *d : Devices) {
        if (d->udid() == udid) {
            m_currentDevice = d;
            emit currentDeviceChanged();
            emit storageSyncChanged();
            emit backupChanged();
            emit softwareChanged();
            checkSoftwareUpdates(true);
            return;
        }
    }
}

QVariantList iDeviceWatcher::getModel()
{
    QVariantList model;

    if (Devices.isEmpty()) {
        QVariantMap element;
        element["image"] = "/images/iphone.png";
        element["device_name"] = "No device";
        element["udid"] = "";
        element["product_type"] = "";
        element["device_class"] = "";
        element["marketing_name"] = "";
        element["battery_string"] = "0";
        element["battery"] = 0;
        model.prepend(element);
    } else {
        for (auto *d : Devices) {
            QVariantMap element;
            element["image"] = d->device_image();
            element["device_name"] = d->device_name();
            element["udid"] = d->udid();
            element["product_type"] = d->product_type();
            element["device_class"] = d->device_class();
            element["marketing_name"] = d->marketing_name();
            element["battery_string"] = QString::number(d->battery());
            element["battery"] = d->battery();
            model.prepend(element);
        }
    }

    return model;
}

void iDeviceWatcher::startStorageSync()
{
    if (m_currentDevice)
        m_currentDevice->startStorageSync();
}

void iDeviceWatcher::startBackup(const QString &path, bool enableEncryption, const QString &password)
{
    if (m_currentDevice)
        m_currentDevice->startBackup(path, enableEncryption, password);
}

void iDeviceWatcher::stopBackup()
{
    if (m_currentDevice)
        m_currentDevice->stopBackup();
}

void iDeviceWatcher::disableBackupEncryption(const QString &path, const QString &password)
{
    if (m_currentDevice)
        m_currentDevice->disableBackupEncryption(path, password);
}

void iDeviceWatcher::changeBackupPassword(const QString &path, const QString &oldPassword, const QString &newPassword)
{
    if (m_currentDevice)
        m_currentDevice->changeBackupPassword(path, oldPassword, newPassword);
}

static QString plistStringAtPath(const QString &path, const char *key)
{
    plist_t plist = nullptr;
    const QByteArray bytes = path.toUtf8();
    if (plist_read_from_file(bytes.constData(), &plist, nullptr) != PLIST_ERR_SUCCESS || !plist)
        return {};

    plist_t node = plist_dict_get_item(plist, key);
    char *value = nullptr;
    if (node)
        plist_get_string_val(node, &value);
    const QString result = value ? QString::fromUtf8(value) : QString();
    free(value);
    plist_free(plist);
    return result;
}

static bool plistBoolAtPath(const QString &path, const char *key)
{
    plist_t plist = nullptr;
    const QByteArray bytes = path.toUtf8();
    if (plist_read_from_file(bytes.constData(), &plist, nullptr) != PLIST_ERR_SUCCESS || !plist)
        return false;

    bool result = false;
    plist_t node = plist_dict_get_item(plist, key);
    if (node && plist_get_node_type(node) == PLIST_BOOLEAN) {
        uint8_t value = 0;
        plist_get_bool_val(node, &value);
        result = value != 0;
    }

    plist_free(plist);
    return result;
}

static QDateTime newestModified(std::initializer_list<QFileInfo> files)
{
    QDateTime newest;
    for (const QFileInfo &file : files) {
        if (file.lastModified() > newest)
            newest = file.lastModified();
    }
    return newest;
}

static QString backupDateText(const QDateTime &modified)
{
    return modified.isValid()
        ? QLocale().toString(modified, QStringLiteral("yyyy-MM-dd HH:mm"))
        : QObject::tr("Unknown date");
}

static QVariantMap backupSave(const QString &path, const QDateTime &modified, qint64 size)
{
    QVariantMap save;
    save["date"] = backupDateText(modified);
    save["encrypted"] = plistBoolAtPath(QDir(path).filePath(QStringLiteral("Manifest.plist")), "IsEncrypted");
    save["modified"] = modified.toMSecsSinceEpoch();
    save["name"] = modified.isValid()
        ? QObject::tr("Backup from %1").arg(QLocale().toString(modified, QLocale::ShortFormat))
        : QObject::tr("Backup");
    save["path"] = path;
    save["size"] = size;
    return save;
}

static qint64 backupDirectorySize(const QDir &dir)
{
    qint64 totalSize = 0;
    const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    for (const QFileInfo &entry : entries) {
        if (entry.isFile()) {
            totalSize += entry.size();
        } else if (entry.isDir()) {
            totalSize += backupDirectorySize(QDir(entry.absoluteFilePath()));
        }
    }
    return totalSize;
}

static QString baseUdidForBackupFolder(const QString &folderName)
{
    static const QRegularExpression archiveNamePattern(
        QStringLiteral("^(.*)-\\d{8}-\\d{6}(?:-\\d+)?$"));
    const QRegularExpressionMatch match = archiveNamePattern.match(folderName);
    return match.hasMatch() ? match.captured(1) : folderName;
}

static bool isBackupFolder(const QDir &dir)
{
    return QFileInfo::exists(dir.filePath(QStringLiteral("Manifest.db")))
        || QFileInfo::exists(dir.filePath(QStringLiteral("Status.plist")))
        || QFileInfo::exists(dir.filePath(QStringLiteral("Manifest.plist")));
}

static QVariantList sortedSaves(QVariantList saves)
{
    std::sort(saves.begin(), saves.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("modified")).toLongLong()
            > b.toMap().value(QStringLiteral("modified")).toLongLong();
    });
    return saves;
}

QVariantList iDeviceWatcher::listBackups(const QString &path)
{
    QVariantMap groupedDevices;
    QDir root(path);
    if (path.isEmpty() || !root.exists())
        return {};

    const QFileInfoList backupDirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &backupDirInfo : backupDirs) {
        QDir backupDir(backupDirInfo.absoluteFilePath());
        if (!isBackupFolder(backupDir))
            continue;

        const QString folderName = backupDirInfo.fileName();
        const QString baseUdid = baseUdidForBackupFolder(folderName);
        QVariantMap device = groupedDevices.value(baseUdid).toMap();

        if (device.isEmpty()) {
            const QString infoPath = backupDir.filePath(QStringLiteral("Info.plist"));
            QString displayName = plistStringAtPath(infoPath, "Display Name");
            if (displayName.isEmpty())
                displayName = plistStringAtPath(infoPath, "Device Name");
            if (displayName.isEmpty())
                displayName = baseUdid;

            const QString productType = plistStringAtPath(infoPath, "Product Type");
            const QString marketingName = iDevice::lookup_marketing_name(productType);

            device["name"] = displayName;
            device["label"] = marketingName.isEmpty()
                ? displayName
                : QStringLiteral("%1 (%2)").arg(displayName, marketingName);
            device["udid"] = baseUdid;
            device["saves"] = QVariantList();
        }

        QFileInfo manifest(backupDir.filePath(QStringLiteral("Manifest.db")));
        QFileInfo status(backupDir.filePath(QStringLiteral("Status.plist")));
        QFileInfo manifestPlist(backupDir.filePath(QStringLiteral("Manifest.plist")));
        QVariantList saves = device.value(QStringLiteral("saves")).toList();
        saves.append(backupSave(
            backupDirInfo.absoluteFilePath(),
            newestModified({manifest, status, manifestPlist}),
            backupDirectorySize(backupDir)));
        device["saves"] = saves;
        groupedDevices[baseUdid] = device;
    }

    QVariantList devices;
    const QStringList udids = groupedDevices.keys();
    for (const QString &udid : udids) {
        QVariantMap device = groupedDevices.value(udid).toMap();
        device["saves"] = sortedSaves(device.value(QStringLiteral("saves")).toList());
        devices.append(device);
    }

    return devices;
}

bool iDeviceWatcher::deleteBackup(const QString &backupRoot, const QString &path)
{
    const QString rootPath = QDir(backupRoot).canonicalPath();
    const QString backupPath = QFileInfo(path).canonicalFilePath();
    if (rootPath.isEmpty() || backupPath.isEmpty())
        return false;
    if (backupPath == rootPath || !backupPath.startsWith(rootPath + QDir::separator()))
        return false;

    return QDir(backupPath).removeRecursively();
}

bool iDeviceWatcher::openBackup(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir())
        return false;

    return QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
}
void iDeviceWatcher::checkSoftwareUpdates(bool silent)
{
    if (auto *manager = softwareManager(m_currentDevice)) {
        if (silent && (manager->busy() || manager->downloading() || manager->status() != QStringLiteral("idle") || !manager->updateCandidates().isEmpty()))
            return;
        manager->check(m_currentDevice->product_type(),
                       m_currentDevice->product_version(),
                       m_currentDevice->build_version(),
                       silent);
    }
}

void iDeviceWatcher::downloadSoftwareUpdate(int index)
{
    if (auto *manager = softwareManager(m_currentDevice))
        manager->downloadUpdate(index);
}

void iDeviceWatcher::downloadSoftwareRestore(int index)
{
    if (auto *manager = softwareManager(m_currentDevice))
        manager->downloadRestore(index);
}

void iDeviceWatcher::cancelSoftwareDownload()
{
    if (auto *manager = softwareManager(m_currentDevice))
        manager->cancelDownload();
}

bool iDeviceWatcher::softwareBusy() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->busy() : false;
}

bool iDeviceWatcher::softwareDownloading() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->downloading() : false;
}

double iDeviceWatcher::softwareDownloadProgress() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->downloadProgress() : 0;
}

QString iDeviceWatcher::softwareStatus() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->status() : QStringLiteral("idle");
}

QString iDeviceWatcher::softwareError() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->error() : QString();
}

QVariantList iDeviceWatcher::softwareUpdateCandidates() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->updateCandidates() : QVariantList();
}

QVariantList iDeviceWatcher::softwareRestoreCandidates() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->restoreCandidates() : QVariantList();
}

QString iDeviceWatcher::softwareDownloadedPath() const
{
    auto *manager = softwareManager(m_currentDevice);
    return manager ? manager->downloadedPath() : QString();
}

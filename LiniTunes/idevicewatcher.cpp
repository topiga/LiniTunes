#include "idevicewatcher.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDesktopServices>
#include <QLocale>
#include <QRegularExpression>
#include <QSettings>
#include <QUrl>
#include <plist/plist.h>
#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <idevice++/usbmuxd.hpp>

extern "C" {
#include <idevice.h>
}

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

    while (m_running) {
        UsbmuxdConnectionHandle *conn = nullptr;
        IdeviceFfiError *err = idevice_usbmuxd_new_default_connection(0, &conn);
        if (err) {
            idevice_error_free(err);
            retryDelay(m_running);
            continue;
        }

        UsbmuxdListenerHandle *listener = nullptr;
        err = idevice_usbmuxd_listen(conn, &listener);
        if (err) {
            idevice_error_free(err);
            idevice_usbmuxd_connection_free(conn);
            retryDelay(m_running);
            continue;
        }

        while (m_running) {
            bool connect_event = false;
            UsbmuxdDeviceHandle *dev_handle = nullptr;
            uint32_t disconnect_id = 0;

            err = idevice_usbmuxd_listener_next(
                listener, &connect_event, &dev_handle, &disconnect_id);

            if (err) {
                idevice_error_free(err);
                break;
            }

            if (!m_running) break;

            if (connect_event && dev_handle) {
                char *c_udid = idevice_usbmuxd_device_get_udid(dev_handle);
                uint32_t dev_id = idevice_usbmuxd_device_get_device_id(dev_handle);
                if (c_udid) {
                    qDebug("Listener: device connected %s (id=%u)", c_udid, dev_id);
                    emit deviceConnected(QString::fromUtf8(c_udid), dev_id);
                    idevice_string_free(c_udid);
                }
                idevice_usbmuxd_device_free(dev_handle);
            } else if (disconnect_id != 0) {
                qDebug("Listener: device disconnected (id=%u)", disconnect_id);
                emit deviceDisconnected(disconnect_id);
            }
        }

        idevice_usbmuxd_listener_handle_free(listener);
        idevice_usbmuxd_connection_free(conn);
        retryDelay(m_running, 10);
    }
}

// ---- DeviceInitWorker -----------------------------------------------------

void DeviceInitWorker::doInit(const QString &udid, uint32_t deviceId)
{
    auto *dev = new iDevice();
    auto addr = IdeviceFFI::UsbmuxdAddr::default_new();

    if (dev->init(udid, deviceId, std::move(addr))) {
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
    m_backupFolder = QSettings(QStringLiteral("LiniTunes"), QStringLiteral("LiniTunes"))
                         .value(QStringLiteral("backup_folder"))
                         .toString();

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

void iDeviceWatcher::connectDeviceSignals(iDevice *dev)
{
    connect(dev, &iDevice::storageSyncChanged,
            this, &iDeviceWatcher::storageSyncChanged);
    connect(dev, &iDevice::backupChanged,
            this, &iDeviceWatcher::backupChanged);
}

void iDeviceWatcher::onDeviceConnected(const QString &udid, uint32_t deviceId)
{
    m_deviceIdToUdid[deviceId] = udid;
    QMetaObject::invokeMethod(m_worker,
        [this, udid, deviceId]() { m_worker->doInit(udid, deviceId); },
        Qt::QueuedConnection);
}

void iDeviceWatcher::onDeviceDisconnected(uint32_t deviceId)
{
    const QString udid = m_deviceIdToUdid.take(deviceId);
    if (!udid.isEmpty())
        removeDeviceByUdid(udid);
}

void iDeviceWatcher::onDeviceInitDone(iDevice *dev)
{
    qDebug("Device: %s | %s | %s",
           qPrintable(dev->device_name()),
           qPrintable(dev->product_type()),
           qPrintable(dev->marketing_name()));

    connectDeviceSignals(dev);
    Devices.append(dev);
    updateLists();
}

void iDeviceWatcher::onDeviceInitFailed(const QString &udid)
{
    qDebug("Device init failed: %s", qPrintable(udid));
}

void iDeviceWatcher::removeDeviceByUdid(const QString &udid)
{
    for (int i = 0; i < Devices.size(); ++i) {
        if (Devices[i]->udid() == udid) {
            if (m_currentDevice == Devices[i])
                m_currentDevice = nullptr;
            delete Devices[i];
            Devices.removeAt(i);
            Devices.squeeze();
            updateLists();
            return;
        }
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
        return;
    }

    for (auto *d : Devices) {
        if (d->udid() == udid) {
            m_currentDevice = d;
            emit currentDeviceChanged();
            emit storageSyncChanged();
            emit backupChanged();
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
#include "idevicewatcher.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QLocale>
#include <plist/plist.h>
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

static QDateTime newestModified(std::initializer_list<QFileInfo> files)
{
    QDateTime newest;
    for (const QFileInfo &file : files) {
        if (file.lastModified() > newest)
            newest = file.lastModified();
    }
    return newest;
}

static QVariantMap backupSave(const QString &path, const QDateTime &modified, qint64 size)
{
    QVariantMap save;
    save["name"] = modified.isValid()
        ? QObject::tr("Backup from %1").arg(QLocale().toString(modified, QLocale::ShortFormat))
        : QObject::tr("Backup");
    save["path"] = path;
    save["size"] = size;
    return save;
}

QVariantList iDeviceWatcher::listBackups(const QString &path)
{
    QVariantList devices;
    QDir root(path);
    if (path.isEmpty() || !root.exists())
        return devices;

    const QFileInfoList deviceDirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &deviceDirInfo : deviceDirs) {
        QDir deviceDir(deviceDirInfo.absoluteFilePath());
        QVariantMap device;
        const QString infoPath = deviceDir.filePath(QStringLiteral("Info.plist"));
        QString displayName = plistStringAtPath(infoPath, "Display Name");
        if (displayName.isEmpty())
            displayName = plistStringAtPath(infoPath, "Device Name");
        if (displayName.isEmpty())
            displayName = deviceDirInfo.fileName();

        device["name"] = displayName;
        device["udid"] = deviceDirInfo.fileName();

        QVariantList saves;
        QFileInfo manifest(deviceDir.filePath(QStringLiteral("Manifest.db")));
        QFileInfo status(deviceDir.filePath(QStringLiteral("Status.plist")));
        QFileInfo manifestPlist(deviceDir.filePath(QStringLiteral("Manifest.plist")));
        if (manifest.exists() || status.exists() || manifestPlist.exists()) {
            saves.append(backupSave(
                deviceDirInfo.absoluteFilePath(),
                newestModified({manifest, status, manifestPlist}),
                manifest.exists() ? manifest.size() : 0));
        }

        const QFileInfoList subDirs = deviceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
        for (const QFileInfo &subDirInfo : subDirs) {
            QFileInfo subManifest(QDir(subDirInfo.absoluteFilePath()).filePath(QStringLiteral("Manifest.db")));
            if (!subManifest.exists())
                continue;
            saves.append(backupSave(
                subDirInfo.absoluteFilePath(),
                subManifest.lastModified(),
                subManifest.size()));
        }

        device["saves"] = saves;
        devices.append(device);
    }

    return devices;
}
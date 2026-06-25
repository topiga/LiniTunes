#include "idevicewatcher.h"
#include <QDebug>
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
    if (m_deviceIdToUdid.contains(deviceId)) {
        QString udid = m_deviceIdToUdid[deviceId];
        m_deviceIdToUdid.remove(deviceId);
        removeDeviceByUdid(udid);
    }
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
        return;
    }

    for (auto *d : Devices) {
        if (d->udid() == udid) {
            m_currentDevice = d;
            emit currentDeviceChanged();
            emit storageSyncChanged();
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
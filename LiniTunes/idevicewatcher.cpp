#include "idevicewatcher.h"

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{
    return;
}

quint8 iDeviceWatcher::begin()
{
    usbmuxd_context = (usbmuxd_subscription_context_t*) malloc(sizeof(usbmuxd_subscription_context_t));
    if (usbmuxd_events_subscribe(usbmuxd_context, (usbmuxd_event_cb_t) CB_devicesChanged, this) != 0) {
        qDebug("Error! Not subscribed to usbmuxd");
        return 1;
    }
    return 0;
}

void iDeviceWatcher::CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *device_watcher)
{
    QString udid = QString(event->device.udid);
    qDebug("Vector size %lld",device_watcher->Devices.size());
    if (event->event == 1) {
        qDebug("Device connected");
        iDevice *new_device = new iDevice(udid.toLatin1().data());
        if (!new_device->device_connected) {
            qDebug("Please approuve this computer and reconnect your device");
            delete new_device;
            return;
        }
        device_watcher->Devices.append(new_device);
        qDebug("Device stored with udid %s", udid.toLatin1().data());
    } else if (event->event == 2) {
        for (qsizetype i = 0; i < device_watcher->Devices.size(); i++) {
            if (QString::compare(device_watcher->Devices.at(i)->udid(), udid) == 0) {
                qDebug("Device disconnected");
                delete device_watcher->Devices.at(i);
                device_watcher->Devices.removeAt(i);
                device_watcher->Devices.squeeze();
                break;
            }
        }
    } else {
        qDebug("Unknown error.");
    }
    device_watcher->updateLists();
}

void iDeviceWatcher::updateLists() {
    this->_serial_list.clear();
    this->_udid_list.clear();
    this->_ecid_list.clear();
    this->_product_type_list.clear();
    this->_device_name_list.clear();
    this->_storage_capacity_list.clear();
    this->_device_image_list.clear();
    for (qsizetype i = 0; i < this->Devices.size(); i++) {
        this->_serial_list.append(this->Devices.at(i)->serial());
        this->_udid_list.append(this->Devices.at(i)->udid());
        this->_ecid_list.append(this->Devices.at(i)->ecid());
        this->_product_type_list.append(this->Devices.at(i)->product_type());
        this->_device_name_list.append(this->Devices.at(i)->device_name());
        this->_storage_capacity_list.append(this->Devices.at(i)->storage_capacity());
        if (QFile::exists(":/images/Devices/"+this->Devices.at(i)->product_type()+".png")) {
            this->_device_image_list.append(QString("/images/Devices/"+this->Devices.at(i)->product_type()+".png"));
        } else if (QFile::exists(":/images/Devices/"+this->Devices.at(i)->device_class()+"Generic.png")) {
            this->_device_image_list.append(QString("/images/Devices/"+this->Devices.at(i)->device_class()+"Generic.png"));
        } else if (QFile::exists(":/images/Devices/Generic.png")) {
            this->_device_image_list.append(QString("/images/Devices/Generic.png"));
        } else {
            this->_device_image_list.append(QString("/images/iDevice/iDevice_90x90.png"));
        }
    }
    if (this->Devices.isEmpty()) {
        this->CurrentDevice = NULL;
        emit currentDeviceChanged();
    } else if (this->Devices.size() == 1) {
        this->CurrentDevice = this->Devices.at(0);
        emit currentDeviceChanged();
    }
    emit ecidListChanged();
}

void iDeviceWatcher::switchCurrentDevice(QString udid)
{
    if (udid.isEmpty()) {
        CurrentDevice = NULL;
        return;
        emit currentDeviceChanged();
    }
    for (qsizetype i = 0; i < this->Devices.size(); i++) {
        if (QString::compare(this->Devices.at(i)->udid(), udid) == 0) {
            CurrentDevice = this->Devices.at(i);
            emit currentDeviceChanged();
            return;
        }
    }
    qDebug("Error! No device matching this udid");
    return;
}

QString iDeviceWatcher::device_image()
{
    if (CurrentDevice) {
        if (QFile::exists(":/images/Devices/"+CurrentDevice->product_type()+".png")) {
            return QString("/images/Devices/"+CurrentDevice->product_type()+".png");
        } else if (QFile::exists(":/images/Devices/"+CurrentDevice->device_class()+"Generic.png")) {
            return QString("/images/Devices/"+CurrentDevice->device_class()+"Generic.png");
        } else if (QFile::exists(":/images/Devices/Generic.png")) {
            return QString("/images/Devices/Generic.png");
        }
    }
    return QString("/images/iDevice/iDevice_90x90.png");
}

bool iDeviceWatcher::device_connected() {
    if (CurrentDevice)
        return true;
    else
        return false;
}

iDeviceWatcher::~iDeviceWatcher()
{
    usbmuxd_events_unsubscribe(*usbmuxd_context);
    for (qsizetype i = 0; i < this->Devices.size(); i++) {
        if (this->Devices.at(i)) {
            this->Devices.removeAt(i);
            this->Devices.squeeze();
        }
    }
}

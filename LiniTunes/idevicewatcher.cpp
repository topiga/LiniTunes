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
                if (QString::compare(device_watcher->CurrentDevice->udid(), udid) == 0) {
                    qDebug("Currrent device disconnected");
                    device_watcher->CurrentDevice = NULL;
                }
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
    this->_udid_list.clear();
    for (qsizetype i = 0; i < this->Devices.size(); i++) {
        this->_udid_list.append(this->Devices.at(i)->udid());
    }
    if (this->Devices.isEmpty()) {
        this->CurrentDevice = NULL;
        emit currentDeviceChanged();
    } else if (this->Devices.size() == 1) {
        this->switchCurrentDevice(this->Devices.at(0)->udid());
        emit currentDeviceChanged();
    } else if (this->CurrentDevice == NULL) {
        this->switchCurrentDevice(this->Devices.at(0)->udid());
        emit currentDeviceChanged();
    }
    emit udidListChanged();
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

bool iDeviceWatcher::device_connected() {
    if (CurrentDevice)
        return true;
    else
        return false;
}

QVariantList iDeviceWatcher::getModel() {
    QVariantList model;
    if (this->Devices.isEmpty()) {
        QVariantMap element;
        element["image"] = "/images/iphone.png";
        element["device_name"] = "No device";
        element["udid"] = "";
        element["product_type"] = "";
        element["battery_string"] = "0";
        element["battery"] = 0;
        model.prepend(element);
    } else {
        for (qsizetype i = 0; i < this->Devices.size(); i++) {
            QVariantMap element;
            element["image"] = this->Devices.at(i)->device_image();
            element["device_name"] = this->Devices.at(i)->device_name();
            element["udid"] = this->Devices.at(i)->udid();
            element["product_type"] = this->Devices.at(i)->product_type();
            element["battery_string"] = QString::number(this->Devices.at(i)->battery());
            element["battery"] = this->Devices.at(i)->battery();
            model.prepend(element);
        }
    }
    for (const QVariant &item : model) {
        QVariantMap map = item.toMap();
        qDebug() << "Item:";
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            qDebug() << "  " << it.key() << ":" << it.value().toString();
        }
    }
    return model;
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

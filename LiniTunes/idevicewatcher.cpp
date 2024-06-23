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

QVariantList iDeviceWatcher::getTestModel()
{
    QVariantList testModel;

    QVariantMap element1;
    element1["image"] = "/images/iphone.png";
    element1["device_name"] = "TF4-iPhone";
    element1["udid"] = "FK1VPUXXJCL8";
    element1["product_type"] = "iPhone10,6";
    element1["battery_string"] = "80";
    element1["battery"] = 80;
    testModel.append(element1);

    QVariantMap element2;
    element2["image"] = "/images/iphone.png";
    element2["device_name"] = "iPhone de Fanny";
    element2["udid"] = "FK2VPUXXJCL8";
    element2["product_type"] = "iPhone10,6";
    element2["battery_string"] = "100";
    element2["battery"] = 100;
    testModel.append(element2);

    QVariantMap element3;
    element3["image"] = "/images/iphone.png";
    element3["device_name"] = "iPhone de Marine";
    element3["udid"] = "FK3VPUXXJCL8";
    element3["product_type"] = "iPhone10,6";
    element3["battery_string"] = "80";
    element3["battery"] = 80;
    testModel.append(element3);

    QVariantMap element4;
    element4["image"] = "/images/iphone.png";
    element4["device_name"] = "iPhone de Arthur";
    element4["udid"] = "FK4VPUXXJCL8";
    element4["product_type"] = "iPhone10,6";
    element4["battery_string"] = "100";
    element4["battery"] = 100;
    testModel.append(element4);

    for (const QVariant &item : testModel) {
        QVariantMap map = item.toMap();
        qDebug() << "Item:";
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            qDebug() << "  " << it.key() << ":" << it.value().toString();
        }
    }

    return testModel;
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

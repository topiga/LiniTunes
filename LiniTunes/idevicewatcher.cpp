#include "idevicewatcher.h"

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{
    usbmuxd_context = (usbmuxd_subscription_context_t*) malloc(sizeof(usbmuxd_subscription_context_t));
    if (usbmuxd_events_subscribe(usbmuxd_context, (usbmuxd_event_cb_t) CB_devicesChanged, this) != 0) {
        qDebug("Error! Not subscribed to usbmuxd");
    }
}

void iDeviceWatcher::CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *user_data)
{
    //user_data->updateDevices(event->device.udid);
    QString udid = event->device.udid;
    usbmuxd_device_info_t* device = (usbmuxd_device_info_t*) malloc(sizeof(usbmuxd_device_info_t));
    qDebug("udid: %s", udid.toLatin1().data());
    qDebug("Vector capacity %lld",user_data->Devices.capacity());
    if (event->event == 1) {
        qDebug("Device connected");
        user_data->Devices.append(new iDevice(udid.toLatin1().data()));
        qDebug("Device stored with udid %s", udid.toLatin1().data());
    } else if (event->event == 2) {
        for (qsizetype i = 0; i < user_data->Devices.size(); i++) {
            if (user_data->Devices.at(i)->udid_str().compare(udid)) {
                qDebug("Device disconnected");
                user_data->Devices.removeAt(i);
                user_data->Devices.squeeze();
                break;
            }
        }
    } else {
        qDebug("Unknown error. Exiting...");
        exit(-99);
    }

    if (device) {
        free(device);
    }
}

iDeviceWatcher::~iDeviceWatcher() {
    usbmuxd_events_unsubscribe(*usbmuxd_context);
    for (qsizetype i = 0; i < this->Devices.size(); i++) {
        if (this->Devices.at(i)) {
            this->Devices.removeAt(i);
            this->Devices.squeeze();
        }
    }
}

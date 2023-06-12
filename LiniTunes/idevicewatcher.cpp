#include "idevicewatcher.h"

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{
//    Devices = new iDevice[16];
//    for (size_t i = 0; i < sizeof(this->Devices)/sizeof(this->Devices)[0]; i++) {
//        this->Devices[i] = NULL;
//    }
    device_list = (usbmuxd_device_info_t**) malloc(16*sizeof(usbmuxd_device_info_t*));
    usbmuxd_context = (usbmuxd_subscription_context_t*) malloc(sizeof(usbmuxd_subscription_context_t));
    if (usbmuxd_events_subscribe(usbmuxd_context, (usbmuxd_event_cb_t) CB_devicesChanged, this) != 0) {
        qDebug("Error! Not subscribed to usbmuxd");
    }
}

void iDeviceWatcher::CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *user_data)
{
    user_data->updateDevices(event->device.udid);
}

void iDeviceWatcher::updateDevices(std::string udid) {
    usbmuxd_device_info_t* device = (usbmuxd_device_info_t*) malloc(sizeof(usbmuxd_device_info_t));
    qDebug("udid: %s", udid.data());
    if (usbmuxd_get_device(udid.data(), device, DEVICE_LOOKUP_USBMUX) != 0) {
        qDebug("Device connected");
//        iDevice* new_iDevice = new iDevice(udid.data());
        Devices.append(new iDevice(udid.data()));
        qDebug("Device stored with udid %s", udid.data());
    } else {
        qDebug("Device disconnected");
        for (qsizetype i = 0; i <= this->Devices.size(); i++) {
            if (this->Devices.at(i)->udid_str() == udid) {
                delete this->Devices.at(i);
                Devices.remove(i);
                break;
            }
        }
    }

    if (device) {
        free(device);
    }
}

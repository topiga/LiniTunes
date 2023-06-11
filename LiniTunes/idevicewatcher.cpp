#include "idevicewatcher.h"

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{

    int err = usbmuxd_events_subscribe((usbmuxd_subscription_context_t*) usbmuxd_context, deviceChanged, (void*) NULL);
    if (err != 0) {
        printf("Not subscribed to usbmuxd\n");
        printf("Error code : %d\n", err);
    }
}

usbmuxd_event_cb_t iDeviceWatcher::deviceChanged(const usbmuxd_event_t *event, void *user_data)
{
    printf("Device changed");
}

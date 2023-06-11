#include "idevicewatcher.h"

iDeviceWatcher::iDeviceWatcher(QObject *parent)
    : QObject{parent}
{
    usbmuxd_context = (usbmuxd_subscription_context_t*) malloc(sizeof(usbmuxd_subscription_context_t));
    if (usbmuxd_events_subscribe(usbmuxd_context, (usbmuxd_event_cb_t) deviceChanged, NULL) != 0) {
        printf("Error! Not subscribed to usbmuxd\n");
    }
}

void iDeviceWatcher::deviceChanged(const usbmuxd_event_t *event, void *user_data)
{
    printf("Device changed\n");
}

#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

#include <usbmuxd.h>
#include "idevice.h"

class iDeviceWatcher : public QObject
{
    Q_OBJECT
public:
    explicit iDeviceWatcher(QObject *parent = nullptr);
    iDevice *Devices[8];
    usbmuxd_subscription_context_t *usbmuxd_context;
    static usbmuxd_event_cb_t deviceChanged(const usbmuxd_event_t *event, void *user_data);
    usbmuxd_event_cb_t deviceChanged_cb;


signals:

};

#endif // IDEVICEWATCHER_H

#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

#include <usbmuxd.h>
#include "idevice.h"
#include <QDebug>

class iDeviceWatcher : public QObject
{
    Q_OBJECT
public:
    explicit iDeviceWatcher(QObject *parent = nullptr);
    ~iDeviceWatcher();
    QVector<iDevice*> Devices;
    usbmuxd_subscription_context_t *usbmuxd_context;
    static void CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *user_data);
};

#endif // IDEVICEWATCHER_H

#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

#include <usbmuxd.h>
#include "idevice.h"
#include <QDebug>
#include <QList>

class iDeviceWatcher : public QObject
{
    Q_OBJECT
public:
    explicit iDeviceWatcher(QObject *parent = nullptr);
    QList<iDevice*> Devices;
    usbmuxd_subscription_context_t *usbmuxd_context;
    static void CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *user_data);
    usbmuxd_device_info_t **device_list = NULL;

    void updateDevices(std::string udid);
};

#endif // IDEVICEWATCHER_H

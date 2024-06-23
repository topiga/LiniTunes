#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

#include <usbmuxd.h>
#include "idevice.h"
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

class iDeviceWatcher : public QObject
{
    Q_OBJECT
    // For all devices
    Q_PROPERTY(QStringList udid_list READ udid_list NOTIFY udidListChanged)
    // For the current one
    Q_PROPERTY(QString udid READ udid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString ecid READ ecid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString serial READ serial NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString imei READ imei NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString product_type READ product_type NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_name READ device_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_capacity READ storage_capacity NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_left READ storage_left NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_image READ device_image NOTIFY currentDeviceChanged)
    Q_PROPERTY(bool device_connected READ device_connected NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString battery_string READ battery_string NOTIFY currentDeviceChanged)
    Q_PROPERTY(int battery READ battery NOTIFY currentDeviceChanged)

public:
    explicit iDeviceWatcher(QObject *parent = nullptr);
    ~iDeviceWatcher();
    Q_INVOKABLE iDevice *CurrentDevice = NULL;
    QVector<iDevice*> Devices;

    // usbmuxd callback for hotplug
    quint8 begin();
    static void CB_devicesChanged(const usbmuxd_event_t *event, iDeviceWatcher *device_watcher);
    usbmuxd_subscription_context_t *usbmuxd_context;

    // Choose the main device in QML
    Q_INVOKABLE void switchCurrentDevice(QString udid = NULL);

    Q_INVOKABLE QVariantList getModel();

    // QML values
    void updateLists();
    QStringList udid_list() { return _udid_list; }

    // For the current device
    QString serial() { return CurrentDevice->serial(); }
    QString udid() { return CurrentDevice->udid(); }
    QString ecid() { return CurrentDevice->ecid(); }
    QString imei() { return CurrentDevice->imei(); }
    QString product_type() { return CurrentDevice->product_type(); }
    QString device_name() { return CurrentDevice->device_name(); }
    QString storage_capacity() { return CurrentDevice->storage_capacity(); }
    QString storage_left() { return CurrentDevice->storage_left(); }
    QString device_image() { return CurrentDevice->device_image(); }
    bool device_connected();
    int battery() { return (CurrentDevice->battery()); }
    QString battery_string() { return QString::number(CurrentDevice->battery()); }

signals:
    void udidListChanged();
    void currentDeviceChanged();

private:
    QStringList _udid_list;
};

#endif // IDEVICEWATCHER_H

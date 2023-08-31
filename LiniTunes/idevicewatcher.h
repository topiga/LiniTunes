#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

#include <usbmuxd.h>
#include "idevice.h"
#include <QDebug>
#include <QFile>
#include <QUrl>

class iDeviceWatcher : public QObject
{
    Q_OBJECT
    // For all devices
    Q_PROPERTY(QStringList serial_list READ serial_list NOTIFY serialListChanged)
    Q_PROPERTY(QStringList udid_list READ udid_list NOTIFY udidListChanged)
    Q_PROPERTY(QStringList ecid_list READ ecid_list NOTIFY ecidListChanged)
    Q_PROPERTY(QStringList product_type_list READ product_type_list NOTIFY productTypeListChanged)
    Q_PROPERTY(QStringList device_name_list READ device_name_list NOTIFY deviceNameListChanged)
    Q_PROPERTY(QStringList storage_capacity_list READ storage_capacity_list NOTIFY storageCapacityListChanged)
    Q_PROPERTY(QStringList device_image_list READ device_image_list NOTIFY deviceImageListChanged)
    // For the current one
    Q_PROPERTY(QString udid READ udid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString ecid READ ecid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString serial READ serial NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString product_type READ product_type NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_name READ device_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_capacity READ storage_capacity NOTIFY currentDeviceChanged)
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

    // QML values
    void updateLists();
    QStringList serial_list() { return _serial_list; }
    QStringList udid_list() { return _udid_list; }
    QStringList ecid_list() { return _ecid_list; }
    QStringList product_type_list() { return _product_type_list; }
    QStringList device_name_list() { return _device_name_list; }
    QStringList storage_capacity_list() { return _storage_capacity_list; }
    QStringList device_image_list() { return _device_image_list; }

    // For the current device
    QString serial() { return CurrentDevice->serial(); }
    QString udid() { return CurrentDevice->udid(); }
    QString ecid() { return CurrentDevice->ecid(); }
    QString product_type() { return CurrentDevice->product_type(); }
    QString device_name() { return CurrentDevice->device_name(); }
    QString storage_capacity() { return CurrentDevice->storage_capacity(); }
    QString device_image();
    bool device_connected();
    int battery() { return (CurrentDevice->battery()/100)*22; }
    QString battery_string() { return QString::number(CurrentDevice->battery()); }

signals:
    void serialListChanged();
    void udidListChanged();
    void ecidListChanged();
    void productTypeListChanged();
    void deviceNameListChanged();
    void storageCapacityListChanged();
    void currentDeviceChanged();
    void deviceImageListChanged();

private:
    QStringList _serial_list;
    QStringList _udid_list;
    QStringList _ecid_list;
    QStringList _product_type_list;
    QStringList _device_name_list;
    QStringList _storage_capacity_list;
    QStringList _device_image_list;
};

#endif // IDEVICEWATCHER_H

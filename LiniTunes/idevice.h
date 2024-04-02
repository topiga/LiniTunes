#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QString>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <plist/plist.h>

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(char* udid, QObject *parent = nullptr);
    ~iDevice();

    // Values
    QString serial() { return QString(_serial); }
    QString udid() { return QString(_udid); }
    QString ecid() { return QString::number(_ecid); }
    QString imei() { return QString(_imei); }
    QString product_type() { return QString(_product_type); }
    QString device_name() { return QString(_device_name); }
    QString storage_capacity();
    QString storage_left();
    QString device_class() { return QString(_device_class); }
    bool device_connected = false;
    int battery() { return _battery_capacity; }

private:
    idevice_t _device = NULL;
    lockdownd_client_t _client = NULL;
    char* _device_name = NULL;
    char* _udid = NULL;
    char* _product_type = NULL;
    char* _device_class = NULL;
    char* _software_version = NULL;
    char* _serial = NULL;
    uint64_t _ecid;
    char* _imei;
    char* _model = NULL;
    uint64_t _storage_capacity;
    uint64_t _storage_left;
    uint64_t _battery_capacity;
    bool _battery_charging;
    void _get_basic_info();

signals:
    void deviceNameChanged();

};

#endif // IDEVICE_H

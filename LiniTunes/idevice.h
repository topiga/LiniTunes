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
    QString udid();
    QString ecid();
    QString product_type();
    QString device_name();
    QString storage_capacity();
    QString device_class();

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
    char* _model = NULL;
    uint64_t _storage_capacity;
    uint64_t _battery_capacity;
    bool _battery_charging;
    void _get_basic_info();

signals:
    void deviceNameChanged();

};

#endif // IDEVICE_H

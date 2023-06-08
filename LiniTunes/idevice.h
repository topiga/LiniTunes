#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <plist/plist++.h>

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(QObject *parent = nullptr);
    ~iDevice();
    char* udid = NULL;
    char* device_name = NULL;
    char* product_type = NULL;
    char* software_version = NULL;
    char* serial = NULL;
    uint64_t ecid = NULL;
    char* model = NULL;

private:
    idevice_t _device = NULL;
    lockdownd_client_t _client = NULL;
    void get_basic_info();

signals:

};

#endif // IDEVICE_H

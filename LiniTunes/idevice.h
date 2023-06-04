#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(QObject *parent = nullptr);
    ~iDevice();
    char* udid;
    char* device_name = NULL;

private:
    idevice_t _device = NULL;
    lockdownd_client_t _client = NULL;

signals:

};

#endif // IDEVICE_H

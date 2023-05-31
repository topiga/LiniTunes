#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <libimobiledevice/libimobiledevice.h>

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(QObject *parent = nullptr);
    static char *uuid;

private:
    idevice_t device = NULL;

signals:

};

#endif // IDEVICE_H

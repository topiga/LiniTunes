#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(QObject *parent = nullptr);

signals:

};

#endif // IDEVICE_H

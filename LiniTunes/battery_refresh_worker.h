#ifndef BATTERY_REFRESH_WORKER_H
#define BATTERY_REFRESH_WORKER_H

#include <QObject>
#include <QString>
#include <cstdint>

class BatteryRefreshWorker : public QObject
{
    Q_OBJECT
public:
    explicit BatteryRefreshWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void refresh(const QString &udid, uint32_t deviceId, const QString &muxAddress,
                 const QString &muxKey, quint64 requestId);

signals:
    void batteryRead(QString udid, QString muxKey, quint64 requestId, int capacity);
};

#endif // BATTERY_REFRESH_WORKER_H

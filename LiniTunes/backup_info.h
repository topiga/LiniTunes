#ifndef BACKUP_INFO_H
#define BACKUP_INFO_H

#include <QObject>
#include <cstdint>

/// Holds backup state and progress for the current device.
class BackupInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(int progress READ progress NOTIFY changed)
    Q_PROPERTY(quint64 bytesDone READ bytesDone NOTIFY changed)
    Q_PROPERTY(quint64 bytesTotal READ bytesTotal NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)

public:
    explicit BackupInfo(QObject *parent = nullptr) : QObject(parent) {}

    enum class Status { Idle, Running, Completed, Failed, Cancelled };
    Q_ENUM(Status)

    void setStatus(Status s);
    void setProgress(quint64 done, quint64 total);
    void setOverallProgress(double pct);
    void setError(const QString &msg);
    void reset();

    QString status() const;
    int progress() const;
    quint64 bytesDone() const { return m_bytesDone; }
    quint64 bytesTotal() const { return m_bytesTotal; }
    double overallPercent() const { return m_overallPercent; }
    QString error() const { return m_error; }

signals:
    void changed();

private:
    Status m_status = Status::Idle;
    quint64 m_bytesDone = 0;
    quint64 m_bytesTotal = 0;
    double m_overallPercent = 0;
    QString m_error;
};

#endif // BACKUP_INFO_H
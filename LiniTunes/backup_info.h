#ifndef BACKUP_INFO_H
#define BACKUP_INFO_H

#include <QObject>
#include <cstdint>

/// Holds backup state and progress for the current device.
class BackupInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString warning READ warning NOTIFY changed)

public:
    explicit BackupInfo(QObject *parent = nullptr) : QObject(parent) {}

    enum class Status { Idle, Running, Completed, CompletedWithWarnings, Failed, Cancelled };
    Q_ENUM(Status)

    void setStatus(Status s);
    void setOverallProgress(double pct);
    void setError(const QString &msg);
    void setWarning(const QString &msg);
    void reset();

    QString status() const;
    double progress() const { return m_overallPercent; }
    QString error() const { return m_error; }
    QString warning() const { return m_warning; }

signals:
    void changed();

private:
    Status m_status = Status::Idle;
    double m_overallPercent = 0;
    QString m_error;
    QString m_warning;
};

#endif // BACKUP_INFO_H
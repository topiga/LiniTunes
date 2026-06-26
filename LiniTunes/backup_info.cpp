#include "backup_info.h"

void BackupInfo::setStatus(Status s)
{
    if (m_status != s) {
        m_status = s;
        emit changed();
    }
}

void BackupInfo::setProgress(quint64 done, quint64 total)
{
    if (m_bytesDone != done || m_bytesTotal != total) {
        m_bytesDone = done;
        m_bytesTotal = total;
        emit changed();
    }
}

void BackupInfo::setOverallProgress(double pct)
{
    double clamped = qBound(0.0, pct, 100.0);
    if (qAbs(m_overallPercent - clamped) > 0.5) {
        m_overallPercent = clamped;
        emit changed();
    }
}

void BackupInfo::setError(const QString &msg)
{
    m_error = msg;
    m_status = Status::Failed;
    emit changed();
}

void BackupInfo::reset()
{
    m_status = Status::Idle;
    m_bytesDone = 0;
    m_bytesTotal = 0;
    m_error.clear();
    emit changed();
}

QString BackupInfo::status() const
{
    switch (m_status) {
    case Status::Idle:      return QStringLiteral("idle");
    case Status::Running:   return QStringLiteral("running");
    case Status::Completed: return QStringLiteral("completed");
    case Status::Failed:    return QStringLiteral("failed");
    case Status::Cancelled: return QStringLiteral("cancelled");
    }
    return QStringLiteral("idle");
}

int BackupInfo::progress() const
{
    return static_cast<int>(m_overallPercent);
}
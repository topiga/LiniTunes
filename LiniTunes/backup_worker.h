#ifndef BACKUP_WORKER_H
#define BACKUP_WORKER_H

#include <QObject>
#include <QString>
#include <atomic>
#include <cstdint>

/// Worker that performs device backup on a background thread.
/// Uses MobileBackup2 delegate callbacks for filesystem I/O.
class BackupWorker : public QObject
{
    Q_OBJECT
public:
    explicit BackupWorker(QObject *parent = nullptr);

public slots:
    void runBackup(const QString &udid, uint32_t deviceId, const QString &backupPath);
    void cancel();

signals:
    void progress(quint64 bytesDone, quint64 bytesTotal, double overall);
    void finished();
    void failed(QString error);
    void cancelled();

private:
    std::atomic<bool> m_cancelled{false};
};

#endif // BACKUP_WORKER_H
#ifndef BACKUP_WORKER_H
#define BACKUP_WORKER_H

#include <QObject>
#include <QString>
#include <atomic>
#include <cstdint>

class QProcess;

/// Worker that performs device backup on a background thread.
/// Uses MobileBackup2 delegate callbacks for filesystem I/O.
class BackupWorker : public QObject
{
    Q_OBJECT
public:
    explicit BackupWorker(QObject *parent = nullptr);

public slots:
    void runBackup(const QString &udid, uint32_t deviceId, const QString &backupPath,
                   bool enableEncryption = false, const QString &password = QString());
    void disableEncryption(const QString &udid, uint32_t deviceId, const QString &backupPath,
                           const QString &password);
    void changeEncryptionPassword(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                  const QString &oldPassword, const QString &newPassword);
    void cancel();

signals:
    void progress(quint64 bytesDone, quint64 bytesTotal, double overall);
    void encryptionEnabled();
    void encryptionDisabled();
    void encryptionPasswordChanged();
    void encryptionFailed(QString error);
    void finished();
    void finishedWithWarnings(QString warning);
    void failed(QString error);
    void cancelled();

private:
    bool runIdevicebackup2(const QString &udid, const QString &backupPath);
    void runDirectMobileBackup2(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                bool enableEncryption, const QString &password);
    bool runPasswordChange(const QString &udid, uint32_t deviceId, const QString &backupPath,
                           const QString &oldPassword, const QString &newPassword,
                           QString *error);

    std::atomic<bool> m_cancelled{false};
    QProcess *m_process = nullptr;
};

#endif // BACKUP_WORKER_H
#ifndef LINTUNES_IDEVICE_H
#define LINTUNES_IDEVICE_H

#include <QObject>
#include <QThread>
#include <QString>
#include "storage_info.h"
#include "storage_sync_worker.h"
#include "backup_info.h"
#include "backup_worker.h"

// Forward-declare
namespace IdeviceFFI {
    class UsbmuxdAddr;
}

class iDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* storage_info READ storageInfo CONSTANT)
    Q_PROPERTY(bool storage_syncing READ storageSyncing NOTIFY storageSyncChanged)
    Q_PROPERTY(int storage_sync_progress READ storageSyncProgress NOTIFY storageSyncChanged)

public:
    explicit iDevice(QObject *parent = nullptr);
    ~iDevice() override;

    bool init(const QString &udid, uint32_t deviceId, IdeviceFFI::UsbmuxdAddr &&addr);

    // Device info
    QString serial() const { return m_serial; }
    QString udid() const { return m_udid; }
    QString ecid() const { return m_ecid; }
    QString imei() const { return m_imei; }
    QString product_type() const { return m_productType; }
    QString device_name() const { return m_deviceName; }
    QString storage_capacity() const { return m_storageCapacity; }
    QString storage_left() const { return m_storageLeft; }
    QString device_class() const { return m_deviceClass; }
    QString device_image() const;
    QString marketing_name() const { return m_marketingName; }
    int battery() const { return m_batteryCapacity; }
    bool device_connected() const { return m_connected; }
    uint32_t deviceId() const { return m_deviceId; }

    // Storage
    StorageInfo *storageInfo() const { return m_storageInfo; }
    Q_INVOKABLE void startStorageSync();
    bool storageSyncing() const { return m_storageInfo ? m_storageInfo->syncing() : false; }
    int storageSyncProgress() const { return m_storageSyncProgress; }

    // Backup
    BackupInfo *backupInfo() const { return m_backupInfo; }
    Q_INVOKABLE void startBackup(const QString &path);
    Q_INVOKABLE void stopBackup();
    bool backupRunning() const;

    static QString format_bytes(uint64_t bytes, bool decimals=true);

signals:
    void storageSyncChanged();
    void backupChanged();

private slots:
    void onStorageSyncData(uint64_t total, uint64_t free,
                           uint64_t apps, uint64_t audio,
                           uint64_t photos, uint64_t documents,
                           uint64_t other);
    void onStorageSyncFailed(const QString &error);
    void onBackupProgress(quint64 bytesDone, quint64 bytesTotal, double overall);
    void onBackupFinished();
    void onBackupFinishedWithWarnings(const QString &warning);
    void onBackupFailed(const QString &error);
    void onBackupCancelled();

private:
    QString m_udid;
    QString m_productType;
    QString m_deviceClass;
    QString m_deviceName;
    QString m_serial;
    QString m_ecid;
    QString m_imei;
    QString m_marketingName;
    uint32_t m_deviceId = 0;
    uint64_t m_storageCapacityBytes = 0;
    uint64_t m_storageLeftBytes = 0;
    QString m_storageCapacity;
    QString m_storageLeft;
    int m_batteryCapacity = 0;
    bool m_connected = false;

    StorageInfo *m_storageInfo = nullptr;

    QThread m_storageSyncThread;
    StorageSyncWorker *m_storageSyncWorker = nullptr;
    int m_storageSyncProgress = 0;

    BackupInfo *m_backupInfo = nullptr;
    QThread m_backupThread;
    BackupWorker *m_backupWorker = nullptr;
};

#endif // LINTUNES_IDEVICE_H
#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QStringList>
#include <QVariantList>
#include <atomic>
#include "linitunes_device.h"

class StorageInfo;

// ---- Worker: listens for usbmuxd events on a dedicated thread ----

class UsbmuxdListener : public QObject {
    Q_OBJECT
public:
    explicit UsbmuxdListener(QObject *parent = nullptr);
    ~UsbmuxdListener() override;

public slots:
    void run();
    void stop();

signals:
    void deviceConnected(QString udid, uint32_t deviceId);
    void deviceDisconnected(uint32_t deviceId);

private:
    std::atomic<bool> m_running = false;
};

// ---- Worker: initializes iDevice on a background thread ----

class DeviceInitWorker : public QObject {
    Q_OBJECT
public:
    explicit DeviceInitWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doInit(const QString &udid, uint32_t deviceId);

signals:
    void initDone(iDevice *device);
    void initFailed(QString udid);
};

// ---- Main watcher ----

class iDeviceWatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList udid_list READ udid_list NOTIFY udidListChanged)
    Q_PROPERTY(QString udid READ udid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString ecid READ ecid NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString serial READ serial NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString imei READ imei NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString product_type READ product_type NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString product_version READ product_version NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString build_version READ build_version NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_name READ device_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_capacity READ storage_capacity NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_left READ storage_left NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_image READ device_image NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString software_image READ software_image NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString marketing_name READ marketing_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(bool device_connected READ device_connected NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString battery_string READ battery_string NOTIFY currentDeviceChanged)
    Q_PROPERTY(int battery READ battery NOTIFY currentDeviceChanged)
    Q_PROPERTY(QObject* storage_info READ storageInfo NOTIFY storageSyncChanged)
    Q_PROPERTY(bool storage_syncing READ storageSyncing NOTIFY storageSyncChanged)
    Q_PROPERTY(int storage_sync_progress READ storageSyncProgress NOTIFY storageSyncChanged)
    Q_PROPERTY(QObject* backup_info READ backupInfo NOTIFY backupChanged)
    Q_PROPERTY(bool backup_running READ backupRunning NOTIFY backupChanged)
    Q_PROPERTY(QString backup_encryption_status READ backupEncryptionStatus NOTIFY backupChanged)
    Q_PROPERTY(bool backup_encryption_busy READ backupEncryptionBusy NOTIFY backupChanged)
    Q_PROPERTY(QString backup_encryption_error READ backupEncryptionError NOTIFY backupChanged)
    Q_PROPERTY(QString backup_folder READ backup_folder WRITE setBackupFolder NOTIFY backupFolderChanged)

public:
    explicit iDeviceWatcher(QObject *parent = nullptr);
    ~iDeviceWatcher() override;

    void start();

    QVector<iDevice*> Devices;

    Q_INVOKABLE void switchCurrentDevice(const QString &udid = QString());
    Q_INVOKABLE QVariantList getModel();
    Q_INVOKABLE void startStorageSync();
    Q_INVOKABLE void startBackup(const QString &path, bool enableEncryption = false,
                                  const QString &password = QString());
    Q_INVOKABLE void stopBackup();
    Q_INVOKABLE void disableBackupEncryption(const QString &path, const QString &password);
    Q_INVOKABLE void changeBackupPassword(const QString &path, const QString &oldPassword,
                                          const QString &newPassword);
    Q_INVOKABLE QVariantList listBackups(const QString &path);
    Q_INVOKABLE bool deleteBackup(const QString &backupRoot, const QString &path);
    Q_INVOKABLE bool openBackup(const QString &path);

    void updateLists();
    QStringList udid_list() const { return m_udidList; }
    QString backup_folder() const { return m_backupFolder; }
    void setBackupFolder(const QString &folder);

    QString serial() const { return m_currentDevice ? m_currentDevice->serial() : QString(); }
    QString udid() const { return m_currentDevice ? m_currentDevice->udid() : QString(); }
    QString ecid() const { return m_currentDevice ? m_currentDevice->ecid() : QString(); }
    QString imei() const { return m_currentDevice ? m_currentDevice->imei() : QString(); }
    QString product_type() const { return m_currentDevice ? m_currentDevice->product_type() : QString(); }
    QString product_version() const { return m_currentDevice ? m_currentDevice->product_version() : QString(); }
    QString build_version() const { return m_currentDevice ? m_currentDevice->build_version() : QString(); }
    QString device_name() const { return m_currentDevice ? m_currentDevice->device_name() : QString(); }
    QString storage_capacity() const { return m_currentDevice ? m_currentDevice->storage_capacity() : QString(); }
    QString storage_left() const { return m_currentDevice ? m_currentDevice->storage_left() : QString(); }
    QString device_image() const { return m_currentDevice ? m_currentDevice->device_image() : QString(); }
    QString software_image() const { return m_currentDevice ? m_currentDevice->software_image() : QString(); }
    QString marketing_name() const { return m_currentDevice ? m_currentDevice->marketing_name() : QString(); }
    bool device_connected() const { return m_currentDevice != nullptr; }
    int battery() const { return m_currentDevice ? m_currentDevice->battery() : 0; }
    QString battery_string() const { return m_currentDevice ? QString::number(m_currentDevice->battery()) : QStringLiteral("0"); }
    QObject *storageInfo() const { return m_currentDevice ? m_currentDevice->storageInfo() : nullptr; }
    bool storageSyncing() const { return m_currentDevice ? m_currentDevice->storageSyncing() : false; }
    int storageSyncProgress() const { return m_currentDevice ? m_currentDevice->storageSyncProgress() : 0; }
    QObject *backupInfo() const { return m_currentDevice ? m_currentDevice->backupInfo() : nullptr; }
    bool backupRunning() const { return m_currentDevice ? m_currentDevice->backupRunning() : false; }
    QString backupEncryptionStatus() const { return m_currentDevice ? m_currentDevice->backupEncryptionStatus() : QStringLiteral("unknown"); }
    bool backupEncryptionBusy() const { return m_currentDevice ? m_currentDevice->backupEncryptionBusy() : false; }
    QString backupEncryptionError() const { return m_currentDevice ? m_currentDevice->backupEncryptionError() : QString(); }

signals:
    void udidListChanged();
    void currentDeviceChanged();
    void storageSyncChanged();
    void backupChanged();
    void backupFolderChanged();

private slots:
    void onDeviceConnected(const QString &udid, uint32_t deviceId);
    void onDeviceDisconnected(uint32_t deviceId);
    void onDeviceInitDone(iDevice *dev);
    void onDeviceInitFailed(const QString &udid);

private:
    void removeDeviceByUdid(const QString &udid);
    void connectDeviceSignals(iDevice *dev);

    iDevice *m_currentDevice = nullptr;
    QStringList m_udidList;
    QString m_backupFolder;

    QThread m_listenerThread;
    UsbmuxdListener *m_listener = nullptr;

    QThread m_workerThread;
    DeviceInitWorker *m_worker = nullptr;

    // Track deviceId → UDID mapping for disconnect events
    QMap<uint32_t, QString> m_deviceIdToUdid;
};

#endif // IDEVICEWATCHER_H
#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>
#include <QThread>
#include <QHash>
#include <QStringList>
#include <QVariantList>
#include <atomic>
#include "linitunes_device.h"
#include "netmuxd_manager.h"

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
    void deviceConnected(QString udid, uint32_t deviceId, QString muxAddress, bool networkConnection);
    void deviceDisconnected(QString muxAddress, uint32_t deviceId);

private:
    std::atomic<bool> m_running = false;
};

// ---- Worker: initializes iDevice on a background thread ----

class DeviceInitWorker : public QObject {
    Q_OBJECT
public:
    explicit DeviceInitWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doInit(const QString &udid, uint32_t deviceId, const QString &muxAddress,
                bool networkConnection, bool enableWifiSync);

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
    Q_PROPERTY(QString device_class READ device_class NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString product_version READ product_version NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString build_version READ build_version NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_name READ device_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_capacity READ storage_capacity NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString storage_left READ storage_left NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString device_image READ device_image NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString software_image READ software_image NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString marketing_name READ marketing_name NOTIFY currentDeviceChanged)
    Q_PROPERTY(bool device_connected READ device_connected NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString connection_transport READ connectionTransport NOTIFY currentDeviceChanged)
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
    Q_PROPERTY(bool wifi_sync_enabled READ wifiSyncEnabled WRITE setWifiSyncEnabled NOTIFY wifiSyncEnabledChanged)
    Q_PROPERTY(QString wifi_sync_status READ wifiSyncStatus NOTIFY wifiSyncStatusChanged)
    Q_PROPERTY(QString wifi_sync_error READ wifiSyncError NOTIFY wifiSyncStatusChanged)
    Q_PROPERTY(bool software_busy READ softwareBusy NOTIFY softwareChanged)
    Q_PROPERTY(bool software_downloading READ softwareDownloading NOTIFY softwareChanged)
    Q_PROPERTY(double software_download_progress READ softwareDownloadProgress NOTIFY softwareChanged)
    Q_PROPERTY(QString software_status READ softwareStatus NOTIFY softwareChanged)
    Q_PROPERTY(QString software_error READ softwareError NOTIFY softwareChanged)
    Q_PROPERTY(QVariantList software_update_candidates READ softwareUpdateCandidates NOTIFY softwareChanged)
    Q_PROPERTY(QVariantList software_restore_candidates READ softwareRestoreCandidates NOTIFY softwareChanged)
    Q_PROPERTY(QString software_downloaded_path READ softwareDownloadedPath NOTIFY softwareChanged)

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
    Q_INVOKABLE void checkSoftwareUpdates(bool silent = false);
    Q_INVOKABLE void downloadSoftwareUpdate(int index = 0);
    Q_INVOKABLE void downloadSoftwareRestore(int index = 0);
    Q_INVOKABLE void cancelSoftwareDownload();

    void updateLists();
    QStringList udid_list() const { return m_udidList; }
    QString backup_folder() const { return m_backupFolder; }
    void setBackupFolder(const QString &folder);
    bool wifiSyncEnabled() const { return m_wifiSyncEnabled; }
    void setWifiSyncEnabled(bool enabled);
    QString wifiSyncStatus() const { return m_netmuxd.status(); }
    QString wifiSyncError() const { return m_netmuxd.error(); }

    QString serial() const { return m_currentDevice ? m_currentDevice->serial() : QString(); }
    QString udid() const { return m_currentDevice ? m_currentDevice->udid() : QString(); }
    QString ecid() const { return m_currentDevice ? m_currentDevice->ecid() : QString(); }
    QString imei() const { return m_currentDevice ? m_currentDevice->imei() : QString(); }
    QString product_type() const { return m_currentDevice ? m_currentDevice->product_type() : QString(); }
    QString device_class() const { return m_currentDevice ? m_currentDevice->device_class() : QString(); }
    QString product_version() const { return m_currentDevice ? m_currentDevice->product_version() : QString(); }
    QString build_version() const { return m_currentDevice ? m_currentDevice->build_version() : QString(); }
    QString device_name() const { return m_currentDevice ? m_currentDevice->device_name() : QString(); }
    QString storage_capacity() const { return m_currentDevice ? m_currentDevice->storage_capacity() : QString(); }
    QString storage_left() const { return m_currentDevice ? m_currentDevice->storage_left() : QString(); }
    QString device_image() const { return m_currentDevice ? m_currentDevice->device_image() : QString(); }
    QString software_image() const { return m_currentDevice ? m_currentDevice->software_image() : QString(); }
    QString marketing_name() const { return m_currentDevice ? m_currentDevice->marketing_name() : QString(); }
    bool device_connected() const { return m_currentDevice != nullptr; }
    QString connectionTransport() const { return m_currentDevice ? m_currentDevice->connectionTransport() : QString(); }
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
    bool softwareBusy() const;
    bool softwareDownloading() const;
    double softwareDownloadProgress() const;
    QString softwareStatus() const;
    QString softwareError() const;
    QVariantList softwareUpdateCandidates() const;
    QVariantList softwareRestoreCandidates() const;
    QString softwareDownloadedPath() const;

signals:
    void udidListChanged();
    void currentDeviceChanged();
    void storageSyncChanged();
    void backupChanged();
    void backupFolderChanged();
    void wifiSyncEnabledChanged();
    void wifiSyncStatusChanged();
    void softwareChanged();

private slots:
    void onDeviceConnected(const QString &udid, uint32_t deviceId, const QString &muxAddress, bool networkConnection);
    void onDeviceDisconnected(const QString &muxAddress, uint32_t deviceId);
    void onDeviceInitDone(iDevice *dev);
    void onDeviceInitFailed(const QString &udid);

private:
    struct MuxEndpoint {
        QString udid;
        uint32_t deviceId = 0;
        QString muxAddress;
        bool networkConnection = false;
    };

    void removeDeviceByMuxKey(const QString &key);
    void initEndpoint(const MuxEndpoint &endpoint);
    void connectDeviceSignals(iDevice *dev);
    void rememberWifiSyncDevice(const QString &udid);
    MuxEndpoint preferredEndpointForUdid(const QString &udid) const;
    bool shouldSwitchToEndpoint(const iDevice *existing, const MuxEndpoint &candidate) const;
    bool shouldUseInitializedDevice(const iDevice *existing, const iDevice *candidate) const;
    iDevice *deviceForUdid(const QString &udid) const;

    iDevice *m_currentDevice = nullptr;
    QStringList m_udidList;
    QString m_backupFolder;
    QStringList m_wifiSyncKnownUdids;
    QString m_lastWifiSyncUdid;
    bool m_wifiSyncEnabled = true;

    NetmuxdManager m_netmuxd;

    QThread m_listenerThread;
    UsbmuxdListener *m_listener = nullptr;

    QThread m_workerThread;
    DeviceInitWorker *m_worker = nullptr;

    // Track mux endpoint key → endpoint for disconnect/fallback handling.
    QHash<QString, MuxEndpoint> m_muxEndpoints;
};

#endif // IDEVICEWATCHER_H
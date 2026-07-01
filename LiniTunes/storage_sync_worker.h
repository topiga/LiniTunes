#ifndef STORAGE_SYNC_WORKER_H
#define STORAGE_SYNC_WORKER_H

#include <QObject>
#include <QString>
#include <cstdint>
#include <string>

/// Worker that performs the Tier-2 storage sync on a background thread.
/// Connects AFC + installation_proxy, scans filesystem, computes categories.
class StorageSyncWorker : public QObject
{
    Q_OBJECT
public:
    explicit StorageSyncWorker(QObject *parent = nullptr);
    static uint64_t scanDirSize(const std::string &path, void *afcRaw);

public slots:
    void runSync(const QString &udid, uint32_t deviceId);

signals:
    void progress(int percent);
    void syncData(uint64_t total, uint64_t free,
                  uint64_t apps, uint64_t audio,
                  uint64_t photos, uint64_t documents,
                  uint64_t other);
    void failed(QString error);

};

#endif // STORAGE_SYNC_WORKER_H
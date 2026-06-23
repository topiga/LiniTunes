#ifndef STORAGE_SYNC_WORKER_H
#define STORAGE_SYNC_WORKER_H

#include <QObject>
#include <QString>
#include <cstdint>
#include "storage_info.h"

/// Worker that performs the Tier-2 storage sync on a background thread.
/// Connects AFC + installation_proxy, scans filesystem, computes categories.
class StorageSyncWorker : public QObject
{
    Q_OBJECT
public:
    explicit StorageSyncWorker(QObject *parent = nullptr);

public slots:
    void runSync(const QString &udid, uint32_t deviceId);

signals:
    void progress(int percent);
    void finished(StorageInfo *result);
    void failed(QString error);

private:
    static uint64_t scanDirSize(const std::string &path, void *afcRaw);
};

#endif // STORAGE_SYNC_WORKER_H
#ifndef STORAGE_INFO_H
#define STORAGE_INFO_H

#include <QObject>
#include <cstdint>

/// Holds all storage-related data for the current device.
/// Tier 1 (immediate) fields are filled on device connect.
/// Tier 2 (sync) fields are filled after startStorageSync() completes.
class StorageInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal totalGb READ totalGb NOTIFY changed)
    Q_PROPERTY(qreal availableGb READ availableGb NOTIFY changed)
    Q_PROPERTY(qreal usedGb READ usedGb NOTIFY changed)
    Q_PROPERTY(qreal audioGb READ audioGb NOTIFY changed)
    Q_PROPERTY(qreal photosGb READ photosGb NOTIFY changed)
    Q_PROPERTY(qreal appsGb READ appsGb NOTIFY changed)
    Q_PROPERTY(qreal documentsGb READ documentsGb NOTIFY changed)
    Q_PROPERTY(qreal otherGb READ otherGb NOTIFY changed)
    Q_PROPERTY(bool complete READ complete NOTIFY changed)
    Q_PROPERTY(bool syncing READ syncing NOTIFY changed)

public:
    explicit StorageInfo(QObject *parent = nullptr) : QObject(parent) {}

    // Tier 1: set from lockdown disk_usage
    void setImmediate(uint64_t totalBytes, uint64_t availableBytes);

    // Tier 2: set from sync worker
    void setSyncResult(uint64_t afcTotalBytes, uint64_t afcFreeBytes,
                       uint64_t appsBytes,
                       uint64_t audioBytes, uint64_t photosBytes,
                       uint64_t documentsBytes, uint64_t otherBytes);
    void setSyncing(bool s);
    void resetSync();

    static qreal bytesToGb(uint64_t bytes) { return bytes / 1'000'000'000.0; }

    qreal totalGb() const     { return m_totalGb; }
    qreal availableGb() const { return m_availableGb; }
    qreal usedGb() const      { return m_usedGb; }
    qreal audioGb() const     { return m_audioGb; }
    qreal photosGb() const    { return m_photosGb; }
    qreal appsGb() const      { return m_appsGb; }
    qreal documentsGb() const { return m_documentsGb; }
    qreal otherGb() const     { return m_otherGb; }
    bool  complete() const    { return m_complete; }
    bool  syncing() const     { return m_syncing; }

signals:
    void changed();

private:
    qreal m_totalGb = 0;
    qreal m_availableGb = 0;
    qreal m_usedGb = 0;
    qreal m_audioGb = 0;
    qreal m_photosGb = 0;
    qreal m_appsGb = 0;
    qreal m_documentsGb = 0;
    qreal m_otherGb = 0;
    bool  m_complete = false;
    bool  m_syncing = false;
};

#endif // STORAGE_INFO_H
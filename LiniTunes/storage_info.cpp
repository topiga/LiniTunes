#include "storage_info.h"

void StorageInfo::setImmediate(uint64_t totalBytes, uint64_t availableBytes)
{
    m_totalGb = bytesToGb(totalBytes);
    m_availableGb = bytesToGb(availableBytes);
    m_usedGb = m_totalGb - m_availableGb;
    emit changed();
}

void StorageInfo::setSyncResult(uint64_t afcTotalBytes, uint64_t afcFreeBytes,
                                 uint64_t appsBytes,
                                 uint64_t audioBytes, uint64_t photosBytes,
                                 uint64_t documentsBytes, uint64_t otherBytes)
{
    m_totalGb = bytesToGb(afcTotalBytes);
    m_availableGb = bytesToGb(afcFreeBytes);
    m_usedGb = m_totalGb - m_availableGb;
    m_audioGb = bytesToGb(audioBytes);
    m_photosGb = bytesToGb(photosBytes);
    m_appsGb = bytesToGb(appsBytes);
    m_documentsGb = bytesToGb(documentsBytes);
    m_otherGb = bytesToGb(otherBytes);
    m_complete = true;
    m_syncing = false;
    emit changed();
}

void StorageInfo::setSyncing(bool s)
{
    m_syncing = s;
    emit changed();
}

void StorageInfo::resetSync()
{
    m_audioGb = 0;
    m_photosGb = 0;
    m_appsGb = 0;
    m_documentsGb = 0;
    m_otherGb = 0;
    m_complete = false;
    emit changed();
}
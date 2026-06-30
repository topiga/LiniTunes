#include "storage_sync_worker.h"
#include <QDebug>
#include <idevice++/provider.hpp>
#include <idevice++/lockdown.hpp>
#include <idevice++/afc.hpp>
#include <idevice++/installation_proxy.hpp>
#include <plist/plist.h>
#include <cstring>
#include <initializer_list>
#include "storage_info.h"

StorageSyncWorker::StorageSyncWorker(QObject *parent)
    : QObject(parent) {}

static uint64_t plist_dict_get_uint_val(const plist_t dict, const char *key)
{
    plist_t node = plist_dict_get_item(dict, key);
    if (!node || plist_get_node_type(node) != PLIST_UINT)
        return 0;
    uint64_t val = 0;
    plist_get_uint_val(node, &val);
    return val;
}

static uint64_t lockdown_uint(IdeviceFFI::Lockdown &lockdown, const char *key, const char *domain)
{
    auto value = lockdown.get_value(key, domain);
    if (value.is_err())
        return 0;

    plist_t node = value.unwrap();
    uint64_t result = 0;
    if (node && plist_get_node_type(node) == PLIST_UINT)
        plist_get_uint_val(node, &result);
    if (node)
        plist_free(node);
    return result;
}

uint64_t StorageSyncWorker::scanDirSize(const std::string &path, void *afcRaw)
{
    auto *afc = static_cast<IdeviceFFI::AfcClient *>(afcRaw);
    uint64_t total = 0;

    auto entries = afc->list_directory(path);
    if (entries.is_err()) return 0;

    for (const auto &name : entries.unwrap()) {
        if (name == "." || name == "..") continue;

        std::string fullPath = path + "/" + name;
        auto info = afc->get_file_info(fullPath);
        if (info.is_err()) continue;

        auto &fi = info.unwrap();
        if (fi.st_ifmt && std::strcmp(fi.st_ifmt, "S_IFDIR") == 0) {
            total += scanDirSize(fullPath, afcRaw);
        } else {
            total += fi.size;
        }
    }
    return total;
}

static uint64_t afcPathSize(const std::string &path, IdeviceFFI::AfcClient &afc)
{
    auto info = afc.get_file_info(path);
    if (info.is_ok()) {
        auto &fi = info.unwrap();
        if (fi.st_ifmt && std::strcmp(fi.st_ifmt, "S_IFDIR") == 0)
            return StorageSyncWorker::scanDirSize(path, &afc);
        return fi.size;
    }
    return 0;
}

static uint64_t sumAfcPaths(std::initializer_list<const char *> paths, IdeviceFFI::AfcClient &afc)
{
    uint64_t total = 0;
    for (const char *path : paths)
        total += afcPathSize(path, afc);
    return total;
}

void StorageSyncWorker::runSync(const QString &udid, uint32_t deviceId)
{
    emit progress(0);

    // Create provider and lockdown
    auto addr = IdeviceFFI::UsbmuxdAddr::default_new();
    auto prov_result = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes-sync");

    if (prov_result.is_err()) {
        emit failed("Failed to create provider");
        return;
    }
    auto provider = std::move(prov_result.unwrap());

    auto lockdown_result = IdeviceFFI::Lockdown::connect(provider);
    if (lockdown_result.is_err()) {
        emit failed("Failed to connect to lockdown");
        return;
    }
    auto lockdown = std::move(lockdown_result.unwrap());

    auto pf = provider.get_pairing_file();
    if (pf.is_ok()) {
        lockdown.start_session(pf.unwrap());
    } else {
        emit failed("No pairing file");
        return;
    }

    emit progress(10);

    // ---- AFC: get accurate total/free ----
    auto afc_result = IdeviceFFI::AfcClient::connect(provider);
    if (afc_result.is_err()) {
        emit failed("Failed to connect AFC");
        return;
    }
    auto afc = std::move(afc_result.unwrap());

    auto devInfo = afc.get_device_info();
    if (devInfo.is_err()) {
        emit failed("Failed to get AFC device info");
        return;
    }
    auto &di = devInfo.unwrap();
    uint64_t afcTotal = di.total_bytes;
    uint64_t afcFree  = di.free_bytes;

    qDebug("AFC: total=%llu free=%llu block_size=%zu",
           (unsigned long long)afcTotal,
           (unsigned long long)afcFree,
           di.block_size);

    emit progress(20);

    // ---- Installation Proxy: get app sizes with proper attributes ----
    uint64_t appsBytes = 0;
    uint64_t appDataBytes = 0;
    auto ipResult = IdeviceFFI::InstallationProxy::connect(provider);
    if (ipResult.is_ok()) {
        auto ip = std::move(ipResult.unwrap());

        // Build options plist: {"ReturnAttributes": ["StaticDiskUsage", "DynamicDiskUsage"]}
        plist_t opts = plist_new_dict();
        plist_t attrs = plist_new_array();
        plist_array_append_item(attrs, plist_new_string("StaticDiskUsage"));
        plist_array_append_item(attrs, plist_new_string("DynamicDiskUsage"));
        plist_dict_set_item(opts, "ReturnAttributes", attrs);

        auto appsResult = ip.browse(IdeviceFFI::Option<plist_t>(opts));
        if (appsResult.is_ok()) {
            auto apps = appsResult.unwrap();
            for (auto &appNode : apps) {
                appsBytes += plist_dict_get_uint_val(appNode, "StaticDiskUsage");
                appDataBytes += plist_dict_get_uint_val(appNode, "DynamicDiskUsage");
                plist_free(appNode);
            }
        }

        plist_free(opts);
    }

    qDebug("Apps: %llu bytes (%.2f GB), app data: %llu bytes (%.2f GB)",
           (unsigned long long)appsBytes, StorageInfo::bytesToGb(appsBytes),
           (unsigned long long)appDataBytes, StorageInfo::bytesToGb(appDataBytes));

    emit progress(40);

    // ---- AFC: scan media directories ----
    uint64_t audioBytes = sumAfcPaths({
        "/iTunes_Control/Music",
        "/Music",
        "/Podcasts",
        "/Recordings",
        "/Audiobooks",
    }, afc);
    const uint64_t booksBytes = sumAfcPaths({
        "/Books",
    }, afc);
    emit progress(50);
    qDebug("Audio: %llu bytes (%.2f GB), books: %llu bytes (%.2f GB)",
           (unsigned long long)audioBytes, StorageInfo::bytesToGb(audioBytes),
           (unsigned long long)booksBytes, StorageInfo::bytesToGb(booksBytes));

    // Finder's Photos number is closer to user-visible photo assets/library data
    // than to the whole /PhotoData tree. Exclude PhotoData caches, search/analysis
    // metadata, and private/external service sandboxes from the Photos bucket.
    uint64_t photosBytes = sumAfcPaths({
        "/DCIM",
        "/Photos",
        "/PhotoStreamsData",
        "/PhotoData/PhotoCloudSharingData",
        "/PhotoData/CPLAssets",
        "/PhotoData/CPL",
        "/PhotoData/Mutations",
        "/PhotoData/Thumbnails",
        "/PhotoData/internal",
        "/PhotoData/AlbumsMetadata",
        "/PhotoData/FacesMetadata",
        "/PhotoData/CameraMetadata",
        "/PhotoData/Journals",
        "/PhotoData/MISC",
        "/PhotoData/Photos.sqlite",
        "/PhotoData/Photos.sqlite-wal",
        "/PhotoData/Photos.sqlite-shm",
    }, afc);
    emit progress(60);
    qDebug("Photos: %llu bytes (%.2f GB)", (unsigned long long)photosBytes,
           StorageInfo::bytesToGb(photosBytes));

    emit progress(70);

    // ---- Compute Documents & Other as remainder ----
    // Use AFC total/free for the completed sync. It matches the visible media
    // partition and avoids expensive recursive scans of cache/support folders.
    const uint64_t totalBytes = afcTotal;
    const uint64_t freeBytes = afcFree;
    const uint64_t usedBytes = totalBytes > freeBytes ? totalBytes - freeBytes : 0;
    const uint64_t systemBytes = lockdown_uint(lockdown, "TotalSystemCapacity", "com.apple.disk_usage");

    uint64_t documentsBytes = appDataBytes + booksBytes;
    const uint64_t otherBytes = systemBytes;
    const uint64_t knownBytes = appsBytes + appDataBytes + booksBytes + audioBytes + photosBytes + otherBytes;
    if (usedBytes > knownBytes)
        documentsBytes += usedBytes - knownBytes;

    qDebug("Other/system: %llu bytes (%.2f GB)",
           (unsigned long long)otherBytes, StorageInfo::bytesToGb(otherBytes));

    // Build result
    emit progress(100);
    emit syncData(totalBytes, freeBytes, appsBytes, audioBytes,
                  photosBytes, documentsBytes, otherBytes);
}
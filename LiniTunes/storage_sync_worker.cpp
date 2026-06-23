#include "storage_sync_worker.h"
#include <QDebug>
#include <idevice++/provider.hpp>
#include <idevice++/lockdown.hpp>
#include <idevice++/afc.hpp>
#include <idevice++/installation_proxy.hpp>
#include <plist/plist.h>
#include <cstring>

StorageSyncWorker::StorageSyncWorker(QObject *parent)
    : QObject(parent) {}

static uint64_t plist_dict_get_uint_val(const plist_t dict, const char *key)
{
    plist_t node = plist_dict_get_item(dict, key);
    if (!node) return 0;
    uint64_t val = 0;
    plist_get_uint_val(node, &val);
    return val;
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

    // ---- Installation Proxy: get app sizes ----
    auto ipResult = IdeviceFFI::InstallationProxy::connect(provider);
    if (ipResult.is_err()) {
        emit failed("Failed to connect installation_proxy");
        return;
    }
    auto ip = std::move(ipResult.unwrap());

    auto appsResult = ip.get_apps(
        IdeviceFFI::Option<std::string>(),          // any type
        IdeviceFFI::Option<std::vector<std::string>>() // all bundles
    );

    uint64_t appsBytes = 0;
    if (appsResult.is_ok()) {
        auto apps = appsResult.unwrap();
        for (auto &appNode : apps) {
            uint64_t size = plist_dict_get_uint_val(appNode, "StaticDiskUsage");
            if (size == 0) {
                // Try ApplicationSINF as fallback
                plist_t sinf = plist_dict_get_item(appNode, "ApplicationSINF");
                if (sinf) {
                    size = plist_dict_get_uint_val(sinf, "TotalDiskUsage");
                }
            }
            appsBytes += size;
            plist_free(appNode);
        }
    }

    qDebug("Apps: %llu bytes (%.2f GB)", (unsigned long long)appsBytes,
           StorageInfo::bytesToGb(appsBytes));

    emit progress(40);

    // ---- AFC: scan media directories ----
    uint64_t audioBytes = scanDirSize("/iTunes_Control/Music", &afc);
    emit progress(50);
    qDebug("Audio: %llu bytes (%.2f GB)", (unsigned long long)audioBytes,
           StorageInfo::bytesToGb(audioBytes));

    uint64_t photosBytes = scanDirSize("/DCIM", &afc);
    emit progress(60);
    qDebug("Photos: %llu bytes (%.2f GB)", (unsigned long long)photosBytes,
           StorageInfo::bytesToGb(photosBytes));

    // Additional media
    uint64_t booksBytes = scanDirSize("/Books", &afc);
    uint64_t audiobooksBytes = scanDirSize("/Audiobooks", &afc);
    uint64_t podcastsBytes = scanDirSize("/Podcasts", &afc);
    emit progress(70);

    audioBytes += booksBytes + audiobooksBytes + podcastsBytes;

    // ---- Compute Documents & Other as remainder ----
    uint64_t afcUsed = afcTotal - afcFree;
    // System partition bytes (~8 GB typically, from TotalSystemCapacity)
    uint64_t systemBytes = 0;
    auto sysCap = lockdown.get_value("TotalSystemCapacity", "com.apple.disk_usage");
    if (sysCap.is_ok()) {
        plist_t node = sysCap.unwrap();
        plist_get_uint_val(node, &systemBytes);
        plist_free(node);
    }

    // Known categories
    uint64_t knownBytes = appsBytes + audioBytes + photosBytes;
    uint64_t documentsBytes = 0;
    uint64_t otherBytes = 0;

    // Remaining used space not in known categories
    if (afcUsed > knownBytes + systemBytes) {
        uint64_t uncategorized = afcUsed - knownBytes - systemBytes;
        // Split arbitrarily: half documents, half other
        documentsBytes = uncategorized / 2;
        otherBytes = uncategorized - documentsBytes;
    }

    // Build result
    auto *info = new StorageInfo();
    info->setSyncResult(afcTotal, afcFree, appsBytes, audioBytes,
                        photosBytes, documentsBytes, otherBytes);

    emit progress(100);
    emit finished(info);
}
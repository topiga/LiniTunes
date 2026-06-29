#include "linitunes_device.h"
#include "plist_helpers.h"
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <plist/plist.h>
#include <idevice++/provider.hpp>
#include <idevice++/lockdown.hpp>
#include <idevice++/usbmuxd.hpp>
#include <cstdlib>

using plist_helpers::stringVal;
using plist_helpers::uintStr;
using plist_helpers::boolVal;

static QString existing_resource_path(const QString &path)
{
    return QFile::exists(QStringLiteral(":") + path) ? path : QString();
}

// ---- Product name lookup table --------------------------------------------
struct DeviceNameEntry {
    const char *identifier;
    const char *marketingName;
};

static const DeviceNameEntry deviceNameTable[] = {
    {"iPhone18,5",  "iPhone 17e"},
    {"iPhone18,4",  "iPhone Air"},
    {"iPhone18,1",  "iPhone 17 Pro"},
    {"iPhone18,2",  "iPhone 17 Pro Max"},
    {"iPhone18,3",  "iPhone 17"},
    {"iPhone17,5",  "iPhone 16e"},
    {"iPhone17,1",  "iPhone 16 Pro"},
    {"iPhone17,2",  "iPhone 16 Pro Max"},
    {"iPhone17,3",  "iPhone 16"},
    {"iPhone17,4",  "iPhone 16 Plus"},
    {"iPhone16,1",  "iPhone 15 Pro Max"},
    {"iPhone16,2",  "iPhone 15 Pro"},
    {"iPhone15,4",  "iPhone 15"},
    {"iPhone15,5",  "iPhone 15 Plus"},
    {"iPhone15,2",  "iPhone 14 Pro Max"},
    {"iPhone15,3",  "iPhone 14 Pro"},
    {"iPhone14,7",  "iPhone 14"},
    {"iPhone14,8",  "iPhone 14 Plus"},
    {"iPhone14,2",  "iPhone 13 Pro"},
    {"iPhone14,3",  "iPhone 13 Pro Max"},
    {"iPhone14,4",  "iPhone 13 mini"},
    {"iPhone14,5",  "iPhone 13"},
    {"iPhone13,1",  "iPhone 12 mini"},
    {"iPhone13,2",  "iPhone 12"},
    {"iPhone13,3",  "iPhone 12 Pro"},
    {"iPhone13,4",  "iPhone 12 Pro Max"},
    {"iPhone12,8",  "iPhone SE (2nd)"},
    {"iPhone12,5",  "iPhone 11 Pro Max"},
    {"iPhone12,3",  "iPhone 11 Pro"},
    {"iPhone12,1",  "iPhone 11"},
    {"iPhone11,8",  "iPhone XR"},
    {"iPhone11,6",  "iPhone XS Max"},
    {"iPhone11,2",  "iPhone XS"},
    {"iPhone10,6",  "iPhone X"},
    {"iPhone10,3",  "iPhone X"},
    {"iPhone10,1",  "iPhone 8"},
    {"iPhone10,4",  "iPhone 8"},
    {"iPhone10,2",  "iPhone 8 Plus"},
    {"iPhone10,5",  "iPhone 8 Plus"},
    {"iPhone9,1",   "iPhone 7"},
    {"iPhone9,3",   "iPhone 7"},
    {"iPhone9,2",   "iPhone 7 Plus"},
    {"iPhone9,4",   "iPhone 7 Plus"},
    {"iPhone8,4",   "iPhone SE (1st)"},
    {"iPhone8,1",   "iPhone 6s"},
    {"iPhone8,2",   "iPhone 6s Plus"},
    {"iPhone7,2",   "iPhone 6"},
    {"iPhone7,1",   "iPhone 6 Plus"},
    {"iPhone6,1",   "iPhone 5S"},
    {"iPhone6,2",   "iPhone 5S"},
    {"iPhone5,3",   "iPhone 5C"},
    {"iPhone5,4",   "iPhone 5C"},
    {"iPhone5,1",   "iPhone 5"},
    {"iPhone5,2",   "iPhone 5"},
    {"iPhone4,1",   "iPhone 4S"},
    {"iPhone3,1",   "iPhone 4"},
    {"iPhone3,2",   "iPhone 4"},
    {"iPhone3,3",   "iPhone 4"},
    {"iPhone2,1",   "iPhone 3GS"},
    {"iPhone1,2",   "iPhone 3G"},
    {"iPhone1,1",   "iPhone"},
    {nullptr, nullptr}
};

static QString lookup_marketing_name(const QString &identifier)
{
    for (const auto *e = deviceNameTable; e->identifier; ++e) {
        if (identifier == QLatin1String(e->identifier))
            return QString::fromUtf8(e->marketingName);
    }
    return identifier; // fallback: return the raw identifier
}

// ---- iDevice implementation -----------------------------------------------

iDevice::iDevice(QObject *parent)
    : QObject{parent}
{
    m_storageSyncWorker = new StorageSyncWorker();
    m_storageSyncWorker->moveToThread(&m_storageSyncThread);
    connect(&m_storageSyncThread, &QThread::finished,
            m_storageSyncWorker, &QObject::deleteLater);
    connect(m_storageSyncWorker, &StorageSyncWorker::syncData,
            this, &iDevice::onStorageSyncData);
    connect(m_storageSyncWorker, &StorageSyncWorker::failed,
            this, &iDevice::onStorageSyncFailed);

    // Backup worker setup
    m_backupInfo = new BackupInfo();
    m_backupInfo->moveToThread(QCoreApplication::instance()->thread());
    m_backupWorker = new BackupWorker();
    m_backupWorker->moveToThread(&m_backupThread);
    connect(&m_backupThread, &QThread::finished,
            m_backupWorker, &QObject::deleteLater);
    connect(m_backupWorker, &BackupWorker::progress,
            this, &iDevice::onBackupProgress);
    connect(m_backupWorker, &BackupWorker::encryptionEnabled,
            this, [this]() { finishBackupEncryptionChange(QStringLiteral("enabled"), false); });
    connect(m_backupWorker, &BackupWorker::encryptionDisabled,
            this, [this]() { finishBackupEncryptionChange(QStringLiteral("disabled"), true); });
    connect(m_backupWorker, &BackupWorker::encryptionPasswordChanged,
            this, [this]() { finishBackupEncryptionChange(QStringLiteral("enabled"), true); });
    connect(m_backupWorker, &BackupWorker::encryptionFailed,
            this, &iDevice::failBackupEncryptionChange);
    connect(m_backupWorker, &BackupWorker::finished,
            this, &iDevice::onBackupFinished);
    connect(m_backupWorker, &BackupWorker::finishedWithWarnings,
            this, &iDevice::onBackupFinishedWithWarnings);
    connect(m_backupWorker, &BackupWorker::failed,
            this, &iDevice::onBackupFailed);
    connect(m_backupWorker, &BackupWorker::cancelled,
            this, &iDevice::onBackupCancelled);
}

iDevice::~iDevice()
{
    m_storageSyncThread.quit();
    m_storageSyncThread.wait();
    m_backupThread.quit();
    m_backupThread.wait();
}

bool iDevice::init(const QString &udid, uint32_t deviceId, IdeviceFFI::UsbmuxdAddr &&addr)
{
    m_udid = udid;
    m_deviceId = deviceId;

    auto prov_result = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes");

    if (prov_result.is_err()) {
        qDebug("ERROR: Failed to create usbmuxd provider");
        return false;
    }
    auto provider = std::move(prov_result.unwrap());

    auto lockdown_result = IdeviceFFI::Lockdown::connect(provider);
    if (lockdown_result.is_err()) {
        qDebug("ERROR: Failed to connect to lockdown");
        return false;
    }
    auto lockdown = std::move(lockdown_result.unwrap());

    auto pf_result = provider.get_pairing_file();
    if (pf_result.is_ok()) {
        auto pf = std::move(pf_result.unwrap());
        auto session_result = lockdown.start_session(pf);
        if (session_result.is_err()) {
            qDebug("WARNING: Failed to start TLS session");
        }
    }

    // Get all top-level values
    auto all_values_result = lockdown.get_value(nullptr, nullptr);
    if (all_values_result.is_err()) {
        qDebug("ERROR: Failed to get device values");
        return false;
    }

    plist_t all_dict = all_values_result.unwrap();

    // Parse using raw C plist API (avoids PList::Dictionary memory corruption)
    m_productType       = stringVal(all_dict, "ProductType");
    m_productVersion    = stringVal(all_dict, "ProductVersion");
    m_buildVersion      = stringVal(all_dict, "BuildVersion");
    m_deviceClass       = stringVal(all_dict, "DeviceClass");
    m_deviceName        = stringVal(all_dict, "DeviceName");
    m_serial            = stringVal(all_dict, "SerialNumber");
    m_ecid              = uintStr(all_dict, "UniqueChipID");
    m_imei              = stringVal(all_dict, "InternationalMobileEquipmentIdentity");

    // Lookup marketing name
    m_marketingName = lookup_marketing_name(m_productType);

    // Free the top-level dictionary
    plist_free(all_dict);

    // Backup encryption state
    auto encryption_result = lockdown.get_value("WillEncrypt", "com.apple.mobile.backup");
    if (encryption_result.is_ok()) {
        plist_t node = encryption_result.unwrap();
        m_backupEncryptionStatus = boolVal(node) ? QStringLiteral("enabled") : QStringLiteral("disabled");
        plist_free(node);
    } else {
        m_backupEncryptionStatus = QStringLiteral("unknown");
    }

    // Battery
    auto battery_result = lockdown.get_value("BatteryCurrentCapacity", "com.apple.mobile.battery");
    if (battery_result.is_ok()) {
        plist_t node = battery_result.unwrap();
        uint64_t val = 0;
        plist_get_uint_val(node, &val);
        m_batteryCapacity = static_cast<int>(val);
        plist_free(node);
    }

    // Disk usage & storage info (tier 1 immediate estimate)
    auto disk_cap = lockdown.get_value("TotalDiskCapacity", "com.apple.disk_usage");
    uint64_t totalBytes = 0;
    if (disk_cap.is_ok()) {
        plist_t node = disk_cap.unwrap();
        plist_get_uint_val(node, &totalBytes);
        m_storageCapacityBytes = totalBytes;
        m_storageCapacity = format_bytes(m_storageCapacityBytes, false);
        plist_free(node);
    }

    // AmountDataAvailable is closer to real free space than TotalDataAvailable
    uint64_t availBytes = 0;
    auto amt_avail = lockdown.get_value("AmountDataAvailable", "com.apple.disk_usage");
    if (amt_avail.is_ok()) {
        plist_t node = amt_avail.unwrap();
        plist_get_uint_val(node, &availBytes);
        plist_free(node);
    }
    if (availBytes == 0) {
        // Fallback to TotalDataAvailable
        auto disk_avail = lockdown.get_value("TotalDataAvailable", "com.apple.disk_usage");
        if (disk_avail.is_ok()) {
            plist_t node = disk_avail.unwrap();
            plist_get_uint_val(node, &availBytes);
            plist_free(node);
        }
    }
    m_storageLeftBytes = availBytes;
    m_storageLeft = format_bytes(m_storageLeftBytes);

    // Create tier-1 storage info
    if (!m_storageInfo)
        m_storageInfo = new StorageInfo(this);
    m_storageInfo->setImmediate(totalBytes, availBytes);

    m_connected = true;
    qDebug("Device: %s | %s | %s", qPrintable(m_deviceName),
           qPrintable(m_productType), qPrintable(m_marketingName));
    return true;
}

QString iDevice::format_bytes(uint64_t bytes, bool decimals)
{
    if (bytes >= 1000000000000ULL) {
        uint64_t tb = bytes / 1000000000000ULL;
        if (decimals) {
            uint64_t dec = (bytes / 100000000000ULL) % 10;
            return QString::number(tb) + "." + QString::number(dec) + " TB";
        } else {
            return QString::number(tb) + " TB";
        }
    }
    if (bytes >= 1000000000ULL) {
        uint64_t gb = bytes / 1000000000ULL;
        if (decimals) {
            uint64_t dec = (bytes / 100000000ULL) % 10;
            return QString::number(gb) + "." + QString::number(dec) + " GB";
        } else {
            return QString::number(gb) + " GB";
        }
    }
    if (bytes >= 1000000ULL) {
        uint64_t mb = bytes / 1000000ULL;
        if (decimals) {
            uint64_t dec = (bytes / 100000ULL) % 10;
            return QString::number(mb) + "." + QString::number(dec) + " MB";
        } else {
            return QString::number(mb) + " MB";
        }
    }
    if (bytes >= 1000ULL) {
        uint64_t kb = bytes / 1000ULL;
        if (decimals) {
            uint64_t dec = (bytes / 100ULL) % 10;
            return QString::number(kb) + "." + QString::number(dec) + " kB";
        } else {
            return QString::number(kb) + " kB";
        }
    }
    if (decimals)
        return QString::number(bytes) + " B";
    return QString::number(bytes) + " B";
}

QString iDevice::device_image() const
{
    QString path = ":/images/devices/" + m_productType + ".png";
    if (QFile::exists(path))
        return "/images/devices/" + m_productType + ".png";
    path = ":/images/devices/" + m_deviceClass + "Generic.png";
    if (QFile::exists(path))
        return "/images/devices/" + m_deviceClass + "Generic.png";
    if (QFile::exists(":/images/devices/Generic.png"))
        return "/images/devices/Generic.png";
    return "/images/iphone.png";
}

QString iDevice::software_image() const
{
    const bool isIOS = m_deviceClass == QStringLiteral("iPhone")
        || m_deviceClass == QStringLiteral("iPod");
    const bool isIPadOS = m_deviceClass == QStringLiteral("iPad");
    const QString majorVersion = m_productVersion.section('.', 0, 0);

    if (majorVersion.isEmpty()) {
        if (isIOS) {
            const QString genericIOS = existing_resource_path(QStringLiteral("/images/software/Generic_iOS.png"));
            if (!genericIOS.isEmpty())
                return genericIOS;
        }

        const QString genericAppleOS = existing_resource_path(QStringLiteral("/images/software/Generic_AppleOS.png"));
        return genericAppleOS.isEmpty() ? device_image() : genericAppleOS;
    }

    QString osPrefix;
    if (isIPadOS)
        osPrefix = QStringLiteral("iPadOS");
    else if (isIOS)
        osPrefix = QStringLiteral("iOS");

    if (!osPrefix.isEmpty()) {
        const QString base = QStringLiteral("/images/software/") + osPrefix + majorVersion;
        const QString png = existing_resource_path(base + QStringLiteral(".png"));
        if (!png.isEmpty())
            return png;

        const QString svg = existing_resource_path(base + QStringLiteral(".svg"));
        if (!svg.isEmpty())
            return svg;
    }

    const QString background = existing_resource_path(QStringLiteral("/images/software/Background_AppleOS.png"));
    if (!background.isEmpty())
        return background;

    const QString genericAppleOS = existing_resource_path(QStringLiteral("/images/software/Generic_AppleOS.png"));
    return genericAppleOS.isEmpty() ? device_image() : genericAppleOS;
}

void iDevice::startStorageSync()
{
    if ((m_storageInfo && m_storageInfo->syncing()) || !m_connected)
        return;

    m_storageSyncProgress = 0;
    if (m_storageInfo)
        m_storageInfo->setSyncing(true);
    emit storageSyncChanged();

    if (!m_storageSyncThread.isRunning())
        m_storageSyncThread.start();

    QMetaObject::invokeMethod(m_storageSyncWorker,
        [this]() { m_storageSyncWorker->runSync(m_udid, m_deviceId); },
        Qt::QueuedConnection);
}

void iDevice::onStorageSyncData(uint64_t total, uint64_t free,
                                   uint64_t apps, uint64_t audio,
                                   uint64_t photos, uint64_t documents,
                                   uint64_t other)
{
    if (m_storageInfo)
        m_storageInfo->setSyncResult(total, free, apps, audio, photos, documents, other);
    m_storageSyncProgress = 100;
    emit storageSyncChanged();
}

void iDevice::onStorageSyncFailed(const QString &error)
{
    qDebug("Storage sync failed: %s", qPrintable(error));
    if (m_storageInfo)
        m_storageInfo->setSyncing(false);
    emit storageSyncChanged();
}

void iDevice::startBackup(const QString &path, bool enableEncryption, const QString &password)
{
    if (backupRunning() || !m_connected)
        return;

    m_backupInfo->reset();
    m_backupInfo->setStatus(BackupInfo::Status::Running);
    emit backupChanged();

    if (!m_backupThread.isRunning())
        m_backupThread.start();

    QMetaObject::invokeMethod(m_backupWorker,
        [this, path, enableEncryption, password]() {
            m_backupWorker->runBackup(m_udid, m_deviceId, path, enableEncryption, password);
        },
        Qt::QueuedConnection);
}

void iDevice::stopBackup()
{
    if (m_backupWorker)
        m_backupWorker->cancel();
}

void iDevice::beginBackupEncryptionChange()
{
    m_backupEncryptionBusy = true;
    m_backupEncryptionError.clear();
    emit backupChanged();
}

void iDevice::finishBackupEncryptionChange(const QString &status, bool resetBackupInfo)
{
    m_backupEncryptionStatus = status;
    m_backupEncryptionBusy = false;
    m_backupEncryptionError.clear();
    if (resetBackupInfo)
        m_backupInfo->reset();
    emit backupChanged();
}

void iDevice::failBackupEncryptionChange(const QString &error)
{
    m_backupEncryptionBusy = false;
    m_backupEncryptionError = error;
    m_backupInfo->setError(error);
    emit backupChanged();
}

void iDevice::disableBackupEncryption(const QString &path, const QString &password)
{
    if (!m_connected || !m_backupWorker || password.isEmpty() || m_backupEncryptionBusy || backupRunning())
        return;

    beginBackupEncryptionChange();

    if (!m_backupThread.isRunning())
        m_backupThread.start();

    QMetaObject::invokeMethod(m_backupWorker,
        [this, path, password]() {
            m_backupWorker->disableEncryption(m_udid, m_deviceId, path, password);
        },
        Qt::QueuedConnection);
}

void iDevice::changeBackupPassword(const QString &path, const QString &oldPassword, const QString &newPassword)
{
    if (!m_connected || !m_backupWorker || oldPassword.isEmpty() || newPassword.isEmpty() || m_backupEncryptionBusy || backupRunning())
        return;

    beginBackupEncryptionChange();

    if (!m_backupThread.isRunning())
        m_backupThread.start();

    QMetaObject::invokeMethod(m_backupWorker,
        [this, path, oldPassword, newPassword]() {
            m_backupWorker->changeEncryptionPassword(m_udid, m_deviceId, path, oldPassword, newPassword);
        },
        Qt::QueuedConnection);
}

bool iDevice::backupRunning() const
{
    return m_backupInfo && m_backupInfo->status() == "running";
}

void iDevice::onBackupProgress(double overall)
{
    m_backupInfo->setOverallProgress(overall * 100.0);
}

void iDevice::onBackupFinished()
{
    m_backupInfo->setStatus(BackupInfo::Status::Completed);
    emit backupChanged();
}

void iDevice::onBackupFinishedWithWarnings(const QString &warning)
{
    m_backupInfo->setWarning(warning);
    emit backupChanged();
}

void iDevice::onBackupFailed(const QString &error)
{
    m_backupInfo->setError(error);
    emit backupChanged();
}

void iDevice::onBackupCancelled()
{
    m_backupInfo->setStatus(BackupInfo::Status::Cancelled);
    emit backupChanged();
}
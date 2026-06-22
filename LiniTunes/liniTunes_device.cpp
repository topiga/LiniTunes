#include "liniTunes_device.h"
#include <QDebug>
#include <QFile>
#include <plist/plist.h>
#include <idevice++/provider.hpp>
#include <idevice++/lockdown.hpp>
#include <idevice++/usbmuxd.hpp>

// Helper: get string from plist dictionary using raw C API
static QString plist_dict_get_string(const plist_t dict, const char *key)
{
    if (!dict || plist_get_node_type(dict) != PLIST_DICT)
        return {};
    plist_t node = plist_dict_get_item(dict, key);
    if (!node)
        return {};
    char *val = nullptr;
    plist_get_string_val(node, &val);
    return val ? QString::fromUtf8(val) : QString();
}

// Helper: get uint64 from plist dictionary using raw C API
static QString plist_dict_get_uint_str(const plist_t dict, const char *key)
{
    if (!dict)
        return {};
    plist_t node = plist_dict_get_item(dict, key);
    if (!node)
        return {};
    uint64_t val = 0;
    plist_get_uint_val(node, &val);
    return QString::number(val);
}

// ---- Product name lookup table --------------------------------------------
struct DeviceNameEntry {
    const char *identifier;
    const char *marketingName;
};

static const DeviceNameEntry deviceNameTable[] = {
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
}

iDevice::~iDevice() = default;

bool iDevice::init(const QString &udid, uint32_t deviceId, IdeviceFFI::UsbmuxdAddr &&addr)
{
    m_udid = udid;

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
    m_productType       = plist_dict_get_string(all_dict, "ProductType");
    m_deviceClass       = plist_dict_get_string(all_dict, "DeviceClass");
    m_deviceName        = plist_dict_get_string(all_dict, "DeviceName");
    m_serial            = plist_dict_get_string(all_dict, "SerialNumber");
    m_ecid              = plist_dict_get_uint_str(all_dict, "UniqueChipID");
    m_imei              = plist_dict_get_string(all_dict, "InternationalMobileEquipmentIdentity");

    QString modelNum    = plist_dict_get_string(all_dict, "ModelNumber");
    QString regionInfo  = plist_dict_get_string(all_dict, "RegionInfo");
    m_model             = modelNum + regionInfo;

    QString swVer = plist_dict_get_string(all_dict, "ProductVersion");
    if (swVer.isEmpty())
        swVer = plist_dict_get_string(all_dict, "HumanReadableProductVersionString");

    // Lookup marketing name
    m_marketingName = lookup_marketing_name(m_productType);

    // Free the top-level dictionary
    plist_free(all_dict);

    // Battery
    auto battery_result = lockdown.get_value("BatteryCurrentCapacity", "com.apple.mobile.battery");
    if (battery_result.is_ok()) {
        plist_t node = battery_result.unwrap();
        uint64_t val = 0;
        plist_get_uint_val(node, &val);
        m_batteryCapacity = static_cast<int>(val);
        plist_free(node);
    }

    auto charging_result = lockdown.get_value("BatteryIsCharging", "com.apple.mobile.battery");
    if (charging_result.is_ok()) {
        plist_t node = charging_result.unwrap();
        uint8_t val = 0;
        plist_get_bool_val(node, &val);
        plist_free(node);
    }

    // Disk usage
    auto disk_cap = lockdown.get_value("TotalDiskCapacity", "com.apple.disk_usage");
    if (disk_cap.is_ok()) {
        plist_t node = disk_cap.unwrap();
        plist_get_uint_val(node, &m_storageCapacityBytes);
        m_storageCapacity = format_bytes(m_storageCapacityBytes);
        plist_free(node);
    }

    auto disk_avail = lockdown.get_value("TotalDataAvailable", "com.apple.disk_usage");
    if (disk_avail.is_ok()) {
        plist_t node = disk_avail.unwrap();
        plist_get_uint_val(node, &m_storageLeftBytes);
        m_storageLeft = format_bytes(m_storageLeftBytes);
        plist_free(node);
    }

    m_connected = true;
    qDebug("Device: %s | %s | %s", qPrintable(m_deviceName),
           qPrintable(m_productType), qPrintable(m_marketingName));
    return true;
}

QString iDevice::format_bytes(uint64_t bytes)
{
    if (bytes >= 1000000000000ULL)
        return QString::number(bytes / 1000000000000ULL) + " TB";
    if (bytes >= 1000000000ULL) {
        uint64_t gb = bytes / 1000000000ULL;
        uint64_t dec = (bytes / 100000000ULL) % 10;
        return QString::number(gb) + "." + QString::number(dec) + " GB";
    }
    if (bytes >= 1000000ULL) {
        uint64_t mb = bytes / 1000000ULL;
        uint64_t dec = (bytes / 100000ULL) % 10;
        return QString::number(mb) + "." + QString::number(dec) + " MB";
    }
    if (bytes >= 1000ULL) {
        uint64_t kb = bytes / 1000ULL;
        uint64_t dec = (bytes / 100ULL) % 10;
        return QString::number(kb) + "." + QString::number(dec) + " kB";
    }
    return QString::number(bytes) + " B";
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
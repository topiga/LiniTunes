#ifndef LINTUNES_IDEVICE_H
#define LINTUNES_IDEVICE_H

#include <QObject>
#include <QString>

// Forward-declare namespace types (definitions are in idevice.cpp)
namespace IdeviceFFI {
    class UsbmuxdAddr;
}
typedef void *plist_t;

class iDevice : public QObject
{
    Q_OBJECT
public:
    explicit iDevice(QObject *parent = nullptr);
    ~iDevice() override;

    // Initialize device connection and fetch all info. Returns true on success.
    bool init(const QString &udid, uint32_t deviceId, IdeviceFFI::UsbmuxdAddr &&addr);

    // Values
    QString serial() const { return m_serial; }
    QString udid() const { return m_udid; }
    QString ecid() const { return m_ecid; }
    QString imei() const { return m_imei; }
    QString product_type() const { return m_productType; }
    QString device_name() const { return m_deviceName; }
    QString storage_capacity() const { return m_storageCapacity; }
    QString storage_left() const { return m_storageLeft; }
    QString device_class() const { return m_deviceClass; }
    QString device_image() const;
    QString marketing_name() const { return m_marketingName; }
    int battery() const { return m_batteryCapacity; }
    bool device_connected() const { return m_connected; }
    static QString format_bytes(uint64_t bytes);

private:
    QString m_udid;
    QString m_productType;
    QString m_deviceClass;
    QString m_deviceName;
    QString m_serial;
    QString m_ecid;
    QString m_imei;
    QString m_model;
    QString m_marketingName;
    uint64_t m_storageCapacityBytes = 0;
    uint64_t m_storageLeftBytes = 0;
    QString m_storageCapacity;
    QString m_storageLeft;
    int m_batteryCapacity = 0;
    bool m_connected = false;
};

#endif // LINTUNES_IDEVICE_H
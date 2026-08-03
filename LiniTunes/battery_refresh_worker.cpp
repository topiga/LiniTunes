#include "battery_refresh_worker.h"
#include "usbmuxd_helpers.h"

#include <idevice++/lockdown.hpp>
#include <idevice++/provider.hpp>
#include <plist/plist.h>
#include <utility>

void BatteryRefreshWorker::refresh(const QString &udid, uint32_t deviceId, const QString &muxAddress,
                                   const QString &muxKey, quint64 requestId)
{
    auto addr = usbmuxd_helpers::makeAddr(muxAddress);
    auto providerResult = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes-battery");
    if (providerResult.is_err()) {
        emit batteryRead(udid, muxKey, requestId, -1);
        return;
    }
    auto provider = std::move(providerResult.unwrap());

    auto lockdownResult = IdeviceFFI::Lockdown::connect(provider);
    if (lockdownResult.is_err()) {
        emit batteryRead(udid, muxKey, requestId, -1);
        return;
    }
    auto lockdown = std::move(lockdownResult.unwrap());

    auto pairingFile = provider.get_pairing_file();
    if (pairingFile.is_ok())
        lockdown.start_session(std::move(pairingFile.unwrap()));

    auto batteryResult = lockdown.get_value("BatteryCurrentCapacity", "com.apple.mobile.battery");
    if (batteryResult.is_err()) {
        emit batteryRead(udid, muxKey, requestId, -1);
        return;
    }

    plist_t node = batteryResult.unwrap();
    uint64_t capacity = 0;
    const bool valid = node && plist_get_node_type(node) == PLIST_UINT;
    if (valid)
        plist_get_uint_val(node, &capacity);
    if (node)
        plist_free(node);

    emit batteryRead(udid, muxKey, requestId,
                     valid && capacity <= 100 ? static_cast<int>(capacity) : -1);
}

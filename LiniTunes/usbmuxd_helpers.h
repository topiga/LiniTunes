#ifndef USBMUXD_HELPERS_H
#define USBMUXD_HELPERS_H

#include <QString>
#include <QStringList>
#include <idevice++/usbmuxd.hpp>

namespace usbmuxd_helpers {

QStringList candidateMuxAddresses();
QString muxKey(const QString &muxAddress, uint32_t deviceId);
IdeviceFFI::UsbmuxdAddr makeAddr(const QString &muxAddress);
IdeviceFFI::Result<IdeviceFFI::UsbmuxdConnection, IdeviceFFI::FfiError> connect(const QString &muxAddress, uint32_t tag = 0);

}

#endif // USBMUXD_HELPERS_H

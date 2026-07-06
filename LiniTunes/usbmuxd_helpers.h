#ifndef USBMUXD_HELPERS_H
#define USBMUXD_HELPERS_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <idevice++/usbmuxd.hpp>

namespace usbmuxd_helpers {

struct MuxSource {
    QString address;
    QString key;
    QString displayName;
    bool netmuxd = false;
};

QString defaultNetmuxdAddress();
MuxSource sourceForAddress(const QString &muxAddress);
QVector<MuxSource> candidateMuxSources();
QStringList candidateMuxAddresses();
QString muxKey(const QString &muxAddress, uint32_t deviceId);
QString muxAddressFromKeySource(const QString &keySource);
QString muxDisplayName(const QString &muxAddress);
IdeviceFFI::UsbmuxdAddr makeAddr(const QString &muxAddress);
IdeviceFFI::Result<IdeviceFFI::UsbmuxdConnection, IdeviceFFI::FfiError> connect(const QString &muxAddress, uint32_t tag = 0);

}

#endif // USBMUXD_HELPERS_H

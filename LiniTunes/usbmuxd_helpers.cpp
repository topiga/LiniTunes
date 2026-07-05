#include "usbmuxd_helpers.h"

#include <QProcessEnvironment>
#include <QtGlobal>
#include <cstring>
#include <utility>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace {

constexpr auto kNetmuxdAddress = "127.0.0.1:27015";
constexpr auto kDefaultMuxKeySource = "default";

void appendUnique(QVector<usbmuxd_helpers::MuxSource> *sources, usbmuxd_helpers::MuxSource source)
{
    for (auto &existing : *sources) {
        if (existing.key != source.key)
            continue;
        existing.netmuxd = existing.netmuxd || source.netmuxd;
        return;
    }
    sources->append(std::move(source));
}

void appendUnique(QVector<usbmuxd_helpers::MuxSource> *sources, const QString &address, bool netmuxd = false)
{
    auto source = usbmuxd_helpers::sourceForAddress(address);
    source.netmuxd = netmuxd;
    appendUnique(sources, std::move(source));
}

bool parseTcpAddress(const QString &muxAddress, sockaddr_in *addr)
{
    const int colon = muxAddress.lastIndexOf(QLatin1Char(':'));
    if (colon <= 0 || colon == muxAddress.size() - 1)
        return false;

    bool ok = false;
    const int port = muxAddress.mid(colon + 1).toInt(&ok);
    if (!ok || port <= 0 || port > 65535)
        return false;

    const QByteArray host = muxAddress.left(colon).toLatin1();
    std::memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(static_cast<uint16_t>(port));
    return inet_pton(AF_INET, host.constData(), &addr->sin_addr) == 1;
}

} // namespace

namespace usbmuxd_helpers {

MuxSource sourceForAddress(const QString &muxAddress)
{
    if (muxAddress.isEmpty()) {
        return {
            QString(),
            QString::fromLatin1(kDefaultMuxKeySource),
            QStringLiteral("default"),
            false,
        };
    }

    return { muxAddress, muxAddress, muxAddress, false };
}

QVector<MuxSource> candidateMuxSources()
{
    QVector<MuxSource> sources;
    const QString envAddress = QProcessEnvironment::systemEnvironment()
        .value(QStringLiteral("USBMUXD_SOCKET_ADDRESS"));
    if (!envAddress.isEmpty())
        appendUnique(&sources, envAddress);

    appendUnique(&sources, QString());

    const QString netmuxdOverride = QProcessEnvironment::systemEnvironment()
        .value(QStringLiteral("LINITUNES_NETMUXD_ADDRESS"));
    if (!netmuxdOverride.isEmpty()) {
        appendUnique(&sources, netmuxdOverride, true);
    }
#ifndef Q_OS_MACOS
    else {
        appendUnique(&sources, QString::fromLatin1(kNetmuxdAddress), true);
    }
#endif
    return sources;
}

QStringList candidateMuxAddresses()
{
    QStringList addresses;
    for (const auto &source : candidateMuxSources())
        addresses.append(source.address);
    return addresses;
}

QString muxKey(const QString &muxAddress, uint32_t deviceId)
{
    return QStringLiteral("%1#%2").arg(sourceForAddress(muxAddress).key).arg(deviceId);
}

QString muxAddressFromKeySource(const QString &keySource)
{
    return keySource == QString::fromLatin1(kDefaultMuxKeySource) ? QString() : keySource;
}

QString muxDisplayName(const QString &muxAddress)
{
    return sourceForAddress(muxAddress).displayName;
}

IdeviceFFI::UsbmuxdAddr makeAddr(const QString &muxAddress)
{
    if (muxAddress.isEmpty())
        return IdeviceFFI::UsbmuxdAddr::default_new();

    sockaddr_in addr;
    if (parseTcpAddress(muxAddress, &addr)) {
        auto result = IdeviceFFI::UsbmuxdAddr::tcp_new(reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
        if (result.is_ok())
            return std::move(result.unwrap());
    }

#if defined(__unix__) || defined(__APPLE__)
    auto unixResult = IdeviceFFI::UsbmuxdAddr::unix_new(muxAddress.toStdString());
    if (unixResult.is_ok())
        return std::move(unixResult.unwrap());
#endif

    return IdeviceFFI::UsbmuxdAddr::default_new();
}

IdeviceFFI::Result<IdeviceFFI::UsbmuxdConnection, IdeviceFFI::FfiError> connect(const QString &muxAddress, uint32_t tag)
{
    if (muxAddress.isEmpty())
        return IdeviceFFI::UsbmuxdConnection::default_new(tag);

    sockaddr_in addr;
    if (parseTcpAddress(muxAddress, &addr)) {
        return IdeviceFFI::UsbmuxdConnection::tcp_new(
            reinterpret_cast<const idevice_sockaddr *>(&addr), sizeof(addr), tag);
    }

#if defined(__unix__) || defined(__APPLE__)
    return IdeviceFFI::UsbmuxdConnection::unix_new(muxAddress.toStdString(), tag);
#else
    return IdeviceFFI::UsbmuxdConnection::default_new(tag);
#endif
}

}

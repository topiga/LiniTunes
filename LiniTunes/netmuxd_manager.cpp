#include "netmuxd_manager.h"
#include "usbmuxd_helpers.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace {
constexpr auto kDisableBundledNetmuxd = "LINITUNES_DISABLE_BUNDLED_NETMUXD";
constexpr auto kNetmuxdOverride = "LINITUNES_NETMUXD_ADDRESS";
constexpr auto kNetmuxdPathOverride = "LINITUNES_NETMUXD_HELPER";
}

NetmuxdManager::NetmuxdManager(QObject *parent)
    : QObject(parent)
{
}

NetmuxdManager::~NetmuxdManager()
{
    stop();
}

void NetmuxdManager::ensureRunning()
{
#ifndef Q_OS_LINUX
    return;
#else
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains(QString::fromLatin1(kDisableBundledNetmuxd))
        || !env.value(QString::fromLatin1(kNetmuxdOverride)).isEmpty()) {
        return;
    }

    if (muxResponds(usbmuxd_helpers::defaultNetmuxdAddress()))
        return;

    if (m_process) {
        if (m_process->state() != QProcess::NotRunning)
            return;
        m_process->deleteLater();
        m_process = nullptr;
    }

    const QString program = bundledNetmuxdPath();
    if (program.isEmpty())
        return;

    const bool systemUsbmuxdAvailable = muxResponds(QString());
    QStringList arguments = {
        QStringLiteral("--host"), QStringLiteral("127.0.0.1"),
        QStringLiteral("--port"), QStringLiteral("27015"),
        QStringLiteral("--disable-unix"),
    };

    if (systemUsbmuxdAvailable) {
        arguments << QStringLiteral("--upstream-usbmuxd");
    } else {
        const QString storage = lockdownDir();
        QDir().mkpath(storage);
        arguments << QStringLiteral("--plist-storage") << storage;
    }

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        logOutput(QProcess::StandardOutput);
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        logOutput(QProcess::StandardError);
    });
    connect(m_process, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qDebug("netmuxd: process error %d", static_cast<int>(error));
    });
    connect(m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [](int code, QProcess::ExitStatus status) {
                qDebug("netmuxd: exited with code %d status %d",
                       code, static_cast<int>(status));
            });

    QProcessEnvironment processEnv = env;
    if (!processEnv.contains(QStringLiteral("RUST_LOG")))
        processEnv.insert(QStringLiteral("RUST_LOG"), QStringLiteral("warn"));
    m_process->setProcessEnvironment(processEnv);
    m_process->setProgram(program);
    m_process->setArguments(arguments);
    m_process->start();

    if (!m_process->waitForStarted(3000)) {
        qDebug("netmuxd: failed to start bundled helper at %s", qPrintable(program));
        m_process->deleteLater();
        m_process = nullptr;
        return;
    }

    qDebug("netmuxd: started bundled helper in %s mode at %s",
           systemUsbmuxdAvailable ? "shim" : "standalone",
           qPrintable(usbmuxd_helpers::defaultNetmuxdAddress()));
#endif
}

void NetmuxdManager::stop()
{
#ifndef Q_OS_LINUX
    return;
#else
    if (!m_process)
        return;

    QProcess *process = m_process;
    m_process = nullptr;
    if (process->state() != QProcess::NotRunning) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
            process->waitForFinished(1000);
        }
    }
    delete process;
#endif
}

QString NetmuxdManager::bundledNetmuxdPath() const
{
#ifndef Q_OS_LINUX
    return {};
#else
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString override = env.value(QString::fromLatin1(kNetmuxdPathOverride));
    const QStringList candidates = override.isEmpty()
        ? QStringList{ QCoreApplication::applicationDirPath() + QStringLiteral("/netmuxd") }
        : QStringList{ override };

    for (const QString &path : candidates) {
        const QFileInfo info(path);
        if (info.exists() && info.isFile() && info.isExecutable())
            return info.absoluteFilePath();
    }
    return {};
#endif
}

QString NetmuxdManager::lockdownDir() const
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::home().filePath(QStringLiteral(".local/share/LiniTunes"));
    return QDir(base).filePath(QStringLiteral("lockdown"));
}

bool NetmuxdManager::muxResponds(const QString &address) const
{
    auto result = usbmuxd_helpers::connect(address, 0);
    return result.is_ok();
}

void NetmuxdManager::logOutput(QProcess::ProcessChannel channel)
{
    if (!m_process)
        return;

    const QByteArray data = channel == QProcess::StandardOutput
        ? m_process->readAllStandardOutput()
        : m_process->readAllStandardError();
    for (const QByteArray &line : data.split('\n')) {
        const QString text = QString::fromUtf8(line).trimmed();
        if (!text.isEmpty())
            qDebug().noquote() << QStringLiteral("netmuxd:") << text;
    }
}

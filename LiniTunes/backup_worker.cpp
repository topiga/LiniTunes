#include "backup_worker.h"
#include "backup_validator.h"
#include "plist_helpers.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <idevice++/lockdown.hpp>
#include <idevice++/mobilebackup2.hpp>
#include <idevice++/provider.hpp>
#include <idevice++/usbmuxd.hpp>
#include <fstream>
#include <map>
#include <stdexcept>
#include <cstdlib>
#include <sys/statvfs.h>

namespace {

using plist_helpers::stringVal;
using plist_helpers::setString;
using plist_helpers::setCurrentDate;
using plist_helpers::boolVal;

bool writePlistToFile(plist_t plist, const QString &path)
{
    char *data = nullptr;
    uint32_t len = 0;
    if (plist_to_bin(plist, &data, &len) != PLIST_ERR_SUCCESS || !data || len == 0)
        return false;

    QSaveFile file(path);
    const bool ok = file.open(QIODevice::WriteOnly)
        && file.write(data, static_cast<qint64>(len)) == static_cast<qint64>(len)
        && file.commit();
    free(data);
    return ok;
}

QString canonicalBackupDir(const QString &backupPath, const QString &udid)
{
    return QDir(backupPath).filePath(udid);
}

QString mobileBackupErrorMessage(uint64_t code, const QString &description)
{
    if (code == 105) {
        return QStringLiteral("Not enough free disk space to complete this backup. Free up space on the backup drive or choose another backup folder.");
    }

    if (!description.isEmpty())
        return description;

    return QStringLiteral("Backup error code %1").arg(code);
}

bool hasReadableCanonicalBackup(const QString &backupPath, const QString &udid)
{
    if (!QFileInfo::exists(canonicalBackupDir(backupPath, udid)))
        return false;
    return BackupValidator::validate(backupPath, udid).readable;
}

QString uniqueArchiveBackupDir(const QString &backupPath, const QString &udid)
{
    const QString baseName = udid + QStringLiteral("-")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    QDir root(backupPath);

    QString candidate = root.filePath(baseName);
    for (int suffix = 1; QFileInfo::exists(candidate); ++suffix)
        candidate = root.filePath(QStringLiteral("%1-%2").arg(baseName).arg(suffix));

    return candidate;
}

bool archiveCanonicalBackup(const QString &backupPath, const QString &udid, QString *error)
{
    const QString canonicalDir = canonicalBackupDir(backupPath, udid);
    if (!QFileInfo::exists(canonicalDir))
        return true;

    if (!hasReadableCanonicalBackup(backupPath, udid)) {
        QDir root(backupPath);
        const QString archive = uniqueArchiveBackupDir(backupPath, udid);
        if (root.rename(canonicalDir, archive)) {
            qDebug("BackupWorker: Archived unreadable/invalid backup: %s", qPrintable(archive));
            return true;
        }
        qDebug("BackupWorker: Removing unreadable/invalid backup: %s", qPrintable(canonicalDir));
        QDir(canonicalDir).removeRecursively();
        return true;
    }

    QDir root(backupPath);
    const QString archive = uniqueArchiveBackupDir(backupPath, udid);
    if (root.rename(canonicalDir, archive)) {
        qDebug("BackupWorker: Archived previous backup: %s", qPrintable(archive));
        return true;
    }

    if (error)
        *error = QStringLiteral("Could not archive previous backup before starting a new one.");
    qDebug("BackupWorker: Failed to archive previous backup: %s", qPrintable(canonicalDir));
    return false;
}

void removeCanonicalBackup(const QString &backupPath, const QString &udid)
{
    const QString path = canonicalBackupDir(backupPath, udid);
    if (QFileInfo::exists(path)) {
        qDebug("BackupWorker: Removing cancelled backup directory: %s", qPrintable(path));
        QDir(path).removeRecursively();
    }
}

bool generateInfoPlist(IdeviceFFI::Provider &provider, const QString &udid, const QString &backupPath)
{
    auto lockdownResult = IdeviceFFI::Lockdown::connect(provider);
    if (lockdownResult.is_err()) {
        auto &err = lockdownResult.unwrap_err();
        qDebug("BackupWorker: Could not connect to lockdown for Info.plist metadata: %s", err.message.c_str());
        return false;
    }

    auto lockdown = std::move(lockdownResult.unwrap());
    auto pairingResult = provider.get_pairing_file();
    if (pairingResult.is_ok()) {
        auto pairingFile = std::move(pairingResult.unwrap());
        auto sessionResult = lockdown.start_session(pairingFile);
        if (sessionResult.is_err())
            qDebug("BackupWorker: Could not start lockdown session for Info.plist metadata");
    }

    auto valuesResult = lockdown.get_value(nullptr, nullptr);
    if (valuesResult.is_err()) {
        auto &err = valuesResult.unwrap_err();
        qDebug("BackupWorker: Could not read lockdown values for Info.plist: %s", err.message.c_str());
        return false;
    }

    plist_t values = valuesResult.unwrap();
    const QString deviceName = stringVal(values, "DeviceName");
    const QString productType = stringVal(values, "ProductType");
    const QString productVersion = stringVal(values, "ProductVersion");
    const QString buildVersion = stringVal(values, "BuildVersion");
    const QString serialNumber = stringVal(values, "SerialNumber");
    const QString imei = stringVal(values, "InternationalMobileEquipmentIdentity");
    const QString meid = stringVal(values, "MobileEquipmentIdentifier");

    const QString displayName = deviceName.isEmpty() ? udid : deviceName;
    const QString guid = QUuid::createUuid()
                             .toString(QUuid::WithoutBraces)
                             .remove('-')
                             .toUpper();

    plist_t info = plist_new_dict();
    setString(info, "Build Version", buildVersion);
    setString(info, "Device Name", displayName);
    setString(info, "Display Name", displayName);
    plist_dict_set_item(info, "GUID", plist_new_string(guid.toUtf8().constData()));
    setString(info, "IMEI", imei);
    setString(info, "MEID", meid.isEmpty() && imei.size() >= 14 ? imei.left(14) : meid);
    setString(info, "Product Type", productType);
    setString(info, "Product Version", productVersion);
    setString(info, "Serial Number", serialNumber);
    plist_dict_set_item(info, "Target Identifier", plist_new_string(udid.toUtf8().constData()));
    plist_dict_set_item(info, "Target Type", plist_new_string("Device"));
    plist_dict_set_item(info, "Unique Identifier", plist_new_string(udid.toUtf8().constData()));
    plist_dict_set_item(info, "iTunes Version", plist_new_string("11.2.0"));
    plist_dict_set_item(info, "Installed Applications", plist_new_array());
    plist_dict_set_item(info, "Applications", plist_new_dict());
    plist_dict_set_item(info, "iTunes Files", plist_new_dict());
    plist_dict_set_item(info, "iTunes Settings", plist_new_dict());
    setCurrentDate(info, "Last Backup Date");

    const QString deviceBackupDir = QDir(backupPath).filePath(udid);
    QDir().mkpath(deviceBackupDir);
    const QString infoPath = QDir(deviceBackupDir).filePath(QStringLiteral("Info.plist"));
    const bool ok = writePlistToFile(info, infoPath);
    if (ok) {
        qDebug("BackupWorker: Wrote Info.plist metadata for %s", qPrintable(udid));
    } else {
        qDebug("BackupWorker: Failed to write Info.plist metadata: %s", qPrintable(infoPath));
    }

    plist_free(info);
    plist_free(values);
    return ok;
}

QString findIdevicebackup2()
{
    QString executable = QStandardPaths::findExecutable(QStringLiteral("idevicebackup2"));
    if (!executable.isEmpty())
        return executable;

    const QStringList fallbackPaths = {
        QStringLiteral("/opt/homebrew/bin/idevicebackup2"),
        QStringLiteral("/usr/local/bin/idevicebackup2"),
        QStringLiteral("/usr/bin/idevicebackup2"),
        QStringLiteral("/bin/idevicebackup2")
    };

    for (const QString &path : fallbackPaths) {
        const QFileInfo fi(path);
        if (fi.exists() && fi.isExecutable())
            return path;
    }

    return {};
}

enum class BackupEncryptionState {
    Unknown,
    Disabled,
    Enabled
};

bool matchesRequestedEncryptionState(BackupEncryptionState state, bool encrypted)
{
    return encrypted
        ? state == BackupEncryptionState::Enabled
        : state == BackupEncryptionState::Disabled;
}

IdeviceFFI::Option<std::string> optionalPassword(const QString &password)
{
    return password.isEmpty()
        ? IdeviceFFI::Option<std::string>()
        : IdeviceFFI::Option<std::string>(password.toStdString());
}

BackupEncryptionState queryBackupEncryption(IdeviceFFI::Provider &provider)
{
    auto lockdownResult = IdeviceFFI::Lockdown::connect(provider);
    if (lockdownResult.is_err())
        return BackupEncryptionState::Unknown;

    auto lockdown = std::move(lockdownResult.unwrap());
    auto pairingResult = provider.get_pairing_file();
    if (pairingResult.is_ok()) {
        auto pairingFile = std::move(pairingResult.unwrap());
        auto sessionResult = lockdown.start_session(pairingFile);
        if (sessionResult.is_err())
            qDebug("BackupWorker: Could not start lockdown session for encryption status");
    }

    auto encryptionResult = lockdown.get_value("WillEncrypt", "com.apple.mobile.backup");
    if (encryptionResult.is_err())
        return BackupEncryptionState::Unknown;

    plist_t node = encryptionResult.unwrap();
    const bool encrypted = boolVal(node);
    plist_free(node);
    return encrypted ? BackupEncryptionState::Enabled : BackupEncryptionState::Disabled;
}

BackupEncryptionState queryBackupEncryption(const QString &udid, uint32_t deviceId)
{
    auto addr = IdeviceFFI::UsbmuxdAddr::default_new();
    auto providerResult = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes-backup-status");
    if (providerResult.is_err())
        return BackupEncryptionState::Unknown;

    auto provider = std::move(providerResult.unwrap());
    return queryBackupEncryption(provider);
}

void configureFilesystemDelegate(IdeviceFFI::BackupDelegateCallbacks &delegate,
                                 std::map<std::string, std::ofstream> &openFiles,
                                 std::atomic<bool> &cancelled)
{
    delegate.is_cancelled = [&cancelled]() {
        return cancelled.load();
    };

    delegate.get_free_disk_space = [](const std::string &path) -> uint64_t {
        struct statvfs st{};
        if (statvfs(path.c_str(), &st) == 0)
            return static_cast<uint64_t>(st.f_bavail) * static_cast<uint64_t>(st.f_frsize);
        return 0;
    };

    delegate.open_file_read = [](const std::string &path) -> std::vector<uint8_t> {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f) {
            if (path.find("Status.plist") != std::string::npos ||
                path.find("Manifest.plist") != std::string::npos ||
                path.find("Info.plist") != std::string::npos) {
                qDebug("BackupWorker: No previous backup metadata found: %s", path.c_str());
            } else {
                qDebug("BackupWorker: Requested backup file is missing: %s", path.c_str());
            }
            throw std::runtime_error("file not found: " + path);
        }

        auto size = f.tellg();
        if (size < 0) {
            qDebug("BackupWorker: Could not determine file size: %s", path.c_str());
            throw std::runtime_error("could not determine file size: " + path);
        }

        f.seekg(0);
        std::vector<uint8_t> buf(static_cast<size_t>(size));
        if (size > 0 && !f.read(reinterpret_cast<char*>(buf.data()), size)) {
            qDebug("BackupWorker: Failed reading file: %s", path.c_str());
            throw std::runtime_error("failed reading file: " + path);
        }
        return buf;
    };

    delegate.create_file_write = [&openFiles](const std::string &path) {
        openFiles[path] = std::ofstream(path, std::ios::binary | std::ios::trunc);
    };

    delegate.write_chunk = [&openFiles](const std::string &path, const uint8_t *data, size_t len) {
        auto it = openFiles.find(path);
        if (it != openFiles.end())
            it->second.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(len));
    };

    delegate.close_file = [&openFiles](const std::string &path) {
        auto it = openFiles.find(path);
        if (it != openFiles.end()) {
            it->second.close();
            openFiles.erase(it);
        }
    };

    delegate.create_dir_all = [](const std::string &path) {
        QDir().mkpath(QString::fromStdString(path));
    };

    delegate.remove = [](const std::string &path) {
        QFileInfo fi(QString::fromStdString(path));
        if (fi.isDir())
            QDir(QString::fromStdString(path)).removeRecursively();
        else
            QFile::remove(QString::fromStdString(path));
    };

    delegate.rename = [](const std::string &from, const std::string &to) {
        QFile::rename(QString::fromStdString(from), QString::fromStdString(to));
    };

    delegate.copy = [](const std::string &src, const std::string &dst) {
        QFileInfo fi(QString::fromStdString(src));
        if (fi.isDir())
            QDir().mkpath(QString::fromStdString(dst));
        else
            QFile::copy(QString::fromStdString(src), QString::fromStdString(dst));
    };

    delegate.exists = [](const std::string &path) -> bool {
        return QFile::exists(QString::fromStdString(path));
    };

    delegate.is_dir = [](const std::string &path) -> bool {
        return QFileInfo(QString::fromStdString(path)).isDir();
    };
}
}

BackupWorker::BackupWorker(QObject *parent)
    : QObject(parent) {}

void BackupWorker::cancel()
{
    m_cancelled = true;
    if (m_process && m_process->state() != QProcess::NotRunning)
        m_process->terminate();
}

void BackupWorker::disableEncryption(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                     const QString &password)
{
    QString error;
    if (password.isEmpty()) {
        emit encryptionFailed(QStringLiteral("Current backup password is required."));
        return;
    }

    if (runPasswordChange(udid, deviceId, backupPath, password, QString(), false, &error))
        emit encryptionDisabled();
    else
        emit encryptionFailed(error);
}

void BackupWorker::changeEncryptionPassword(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                            const QString &oldPassword, const QString &newPassword)
{
    QString error;
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        emit encryptionFailed(QStringLiteral("Current and new backup passwords are required."));
        return;
    }

    if (runPasswordChange(udid, deviceId, backupPath, oldPassword, newPassword, true, &error))
        emit encryptionPasswordChanged();
    else
        emit encryptionFailed(error);
}

bool BackupWorker::runPasswordChange(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                     const QString &oldPassword, const QString &newPassword,
                                     bool targetEncrypted, QString *error)
{
    m_cancelled = false;
    qDebug("BackupWorker: Changing backup encryption state for %s", qPrintable(udid));

    QString rootPath = backupPath;
    if (rootPath.isEmpty())
        rootPath = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath(QStringLiteral("backup-encryption"));
    QDir().mkpath(rootPath);

    auto addr = IdeviceFFI::UsbmuxdAddr::default_new();
    auto provResult = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes-backup-password");
    if (provResult.is_err()) {
        if (error) *error = QStringLiteral("Failed to create provider.");
        return false;
    }

    auto provider = std::move(provResult.unwrap());
    auto backupResult = IdeviceFFI::MobileBackup2::connect(provider);
    if (backupResult.is_err()) {
        auto &err = backupResult.unwrap_err();
        if (error) *error = QStringLiteral("Failed to connect to MobileBackup2: %1").arg(QString::fromStdString(err.message));
        return false;
    }

    auto backupClient = std::move(backupResult.unwrap());
    std::map<std::string, std::ofstream> openFiles;
    IdeviceFFI::BackupDelegateCallbacks delegate;
    configureFilesystemDelegate(delegate, openFiles, m_cancelled);

    auto oldOption = optionalPassword(oldPassword);
    auto newOption = optionalPassword(newPassword);

    auto result = backupClient.change_password(rootPath.toStdString(), oldOption, newOption, delegate);
    for (auto &[path, stream] : openFiles)
        stream.close();
    backupClient.disconnect();

    if (result.is_err()) {
        auto &err = result.unwrap_err();
        const QString errMessage = QString::fromStdString(err.message);

        // Some devices close the MobileBackup2 socket after toggling encryption.
        // For enable/disable operations the final lockdown WillEncrypt value is
        // the authoritative state; for password changes it is not enough to
        // prove the new password took effect, so keep reporting the error.
        const bool togglingEncryption = oldPassword.isEmpty() || newPassword.isEmpty();
        if (togglingEncryption && matchesRequestedEncryptionState(queryBackupEncryption(udid, deviceId), targetEncrypted)) {
            qDebug("BackupWorker: MobileBackup2 returned an error after encryption toggle, but final state matches request: %s",
                   qPrintable(errMessage));
            return true;
        }

        if (error) *error = QStringLiteral("Could not change backup encryption: %1").arg(errMessage);
        return false;
    }

    const BackupEncryptionState finalState = queryBackupEncryption(udid, deviceId);
    if (finalState != BackupEncryptionState::Unknown &&
        !matchesRequestedEncryptionState(finalState, targetEncrypted)) {
        if (error) *error = QStringLiteral("The iPhone did not report the requested backup encryption state.");
        return false;
    }

    return true;
}

void BackupWorker::runBackup(const QString &udid, uint32_t deviceId, const QString &backupPath,
                             bool enableEncryption, const QString &password)
{
    m_cancelled = false;

    // Prefer the in-process third_party/idevice MobileBackup2 implementation by
    // default. The external libimobiledevice CLI remains available as an explicit
    // compatibility escape hatch while the direct backend continues to mature.
    const QString requestedBackend = qEnvironmentVariable("LINITUNES_BACKUP_BACKEND").toLower();
    if (requestedBackend == QStringLiteral("idevicebackup2")
        || requestedBackend == QStringLiteral("libimobiledevice")
        || requestedBackend == QStringLiteral("cli")) {
        if (enableEncryption) {
            emit failed(QStringLiteral("Enabling encrypted backups is only supported by the direct MobileBackup2 backend."));
            return;
        }

        if (runIdevicebackup2(udid, backupPath))
            return;

        emit failed(QStringLiteral("idevicebackup2 was not found. Install libimobiledevice tools and try again.\nFedora: sudo dnf install libimobiledevice-utils\nmacOS: brew install libimobiledevice"));
        return;
    }

    qDebug("BackupWorker: using direct third_party/idevice MobileBackup2 backend");
    runDirectMobileBackup2(udid, deviceId, backupPath, enableEncryption, password);
}

bool BackupWorker::runIdevicebackup2(const QString &udid, const QString &backupPath)
{
    const QString executable = findIdevicebackup2();
    if (executable.isEmpty())
        return false;

    QDir().mkpath(backupPath);

    QProcess process;
    m_process = &process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    const QStringList args = { QStringLiteral("-u"), udid,
                               QStringLiteral("backup"), QStringLiteral("--full"), backupPath };

    qDebug("BackupWorker: Starting idevicebackup2: %s %s", qPrintable(executable), qPrintable(args.join(' ')));
    process.start(executable, args);
    if (!process.waitForStarted()) {
        m_process = nullptr;
        emit failed(QStringLiteral("Failed to start idevicebackup2. Install libimobiledevice (Fedora: libimobiledevice-utils, macOS: brew install libimobiledevice)."));
        return true;
    }

    QString output;
    QRegularExpression percentRegex(QStringLiteral("(\\d{1,3})%"));
    while (process.state() != QProcess::NotRunning) {
        process.waitForReadyRead(250);
        const QString chunk = QString::fromLocal8Bit(process.readAll());
        if (!chunk.isEmpty()) {
            output += chunk;
            const auto match = percentRegex.match(chunk);
            if (match.hasMatch()) {
                const int percent = qBound(0, match.captured(1).toInt(), 100);
                emit progress(percent / 100.0);
            }
        }
        if (m_cancelled && process.state() != QProcess::NotRunning)
            process.terminate();
    }
    output += QString::fromLocal8Bit(process.readAll());
    m_process = nullptr;

    if (m_cancelled) {
        emit cancelled();
        return true;
    }

    const BackupValidationResult validation = BackupValidator::validate(backupPath, udid);
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && validation.readable) {
        qDebug("BackupWorker: idevicebackup2 backup completed and metadata validates");
        emit finished();
        return true;
    }

    qDebug("BackupWorker: idevicebackup2 failed exit=%d validation=%s output=%s",
           process.exitCode(), qPrintable(validation.error), qPrintable(output.right(2000)));

    if (validation.readable) {
        emit finishedWithWarnings(QStringLiteral("idevicebackup2 reported a warning, but backup metadata validates."));
        return true;
    }

    emit failed(QStringLiteral("Backup failed or incomplete. %1").arg(validation.error.isEmpty() ? output.right(500) : validation.error));
    return true;
}

void BackupWorker::runDirectMobileBackup2(const QString &udid, uint32_t deviceId, const QString &backupPath,
                                          bool enableEncryption, const QString &password)
{
    m_cancelled = false;
    qDebug("BackupWorker::runBackup(%s, id=%u, path=%s)",
           qPrintable(udid), deviceId, qPrintable(backupPath));

    // Create provider
    auto addr = IdeviceFFI::UsbmuxdAddr::default_new();
    auto prov_result = IdeviceFFI::Provider::usbmuxd_new(
        std::move(addr), 0, udid.toStdString(), deviceId, "LiniTunes-backup");

    if (prov_result.is_err()) {
        qDebug("BackupWorker: Failed to create provider");
        emit failed("Failed to create provider");
        return;
    }
    auto provider = std::move(prov_result.unwrap());
    qDebug("BackupWorker: Provider created");

    QDir().mkpath(backupPath);
    QString archiveError;
    if (!archiveCanonicalBackup(backupPath, udid, &archiveError)) {
        emit failed(archiveError);
        return;
    }

    const BackupEncryptionState encryptionState = queryBackupEncryption(provider);

    bool encryptionEnabledThisRun = false;
    auto withEncryptionNote = [&encryptionEnabledThisRun](const QString &message) {
        if (!encryptionEnabledThisRun)
            return message;
        return message + QStringLiteral("\nEncrypted backups were enabled on this device. Future backups will stay encrypted until disabled with the backup password.");
    };

    if (enableEncryption && encryptionState == BackupEncryptionState::Unknown) {
        emit failed(QStringLiteral("Could not confirm whether encrypted backups are already enabled. Unlock the iPhone and try again."));
        return;
    }

    // Enable encryption via a dedicated connection before connecting for
    // backup. MobileBackup2 commonly closes the socket after toggling
    // encryption, so using a separate client avoids reconnect hacks.
    if (enableEncryption && encryptionState == BackupEncryptionState::Disabled) {
        if (password.isEmpty()) {
            emit failed(QStringLiteral("Encrypted backup requires a password."));
            return;
        }
        QString encError;
        if (!runPasswordChange(udid, deviceId, backupPath, QString(), password, true, &encError)) {
            emit failed(QStringLiteral("Could not enable encrypted backups. Unlock the iPhone, confirm the passcode prompt if shown, and try again."));
            return;
        }
        encryptionEnabledThisRun = true;
        emit encryptionEnabled();
    } else if (enableEncryption) {
        qDebug("BackupWorker: Encrypted backups already enabled");
    }

    // Ensure backup directory exists. The backup library stores data under
    // <backup_root>/<udid>/. If a previous attempt left empty metadata plists,
    // iOS may try to read them as an existing backup and fail with
    // MBErrorDomain/205 (zero-length Status.plist). Remove only clearly
    // incomplete metadata so a fresh backup can start cleanly.
    const QString deviceBackupDir = canonicalBackupDir(backupPath, udid);
    const QStringList metadataFiles = {
        QStringLiteral("Status.plist"),
        QStringLiteral("Manifest.plist"),
        QStringLiteral("Manifest.db")
    };
    bool hasEmptyMetadata = false;
    for (const QString &metadataFile : metadataFiles) {
        const QFileInfo fi(QDir(deviceBackupDir).filePath(metadataFile));
        if (fi.exists() && fi.isFile() && fi.size() == 0) {
            qDebug("BackupWorker: Found empty backup metadata file: %s", qPrintable(fi.filePath()));
            hasEmptyMetadata = true;
        }
    }
    if (hasEmptyMetadata) {
        qDebug("BackupWorker: Removing incomplete backup directory: %s", qPrintable(deviceBackupDir));
        QDir(deviceBackupDir).removeRecursively();
    }

    // Write host-side Info.plist metadata after any previous backup has been
    // archived or cleaned. MobileBackup2 writes Status/Manifest data itself.
    generateInfoPlist(provider, udid, backupPath);

    // Connect to mobilebackup2 service
    auto backup_result = IdeviceFFI::MobileBackup2::connect(provider);
    if (backup_result.is_err()) {
        auto &err = backup_result.unwrap_err();
        qDebug("BackupWorker: Failed to connect to mobilebackup2: %s", err.message.c_str());
        emit failed(withEncryptionNote("Failed to connect to mobilebackup2 service: " +
                     QString::fromStdString(err.message)));
        return;
    }
    auto backup_client = std::move(backup_result.unwrap());
    qDebug("BackupWorker: MobileBackup2 connected");

    auto pf_result = provider.get_pairing_file();
    if (pf_result.is_ok())
        qDebug("BackupWorker: Pairing file obtained");
    else
        qDebug("BackupWorker: WARNING - no pairing file, trust may fail");

    auto validateBackup = [&backupPath, &udid]() {
        return BackupValidator::validate(backupPath, udid);
    };

    std::string backup_root = backupPath.toStdString();
    qDebug("BackupWorker: Backup directory: %s", backup_root.c_str());

    std::map<std::string, std::ofstream> open_files;
    IdeviceFFI::BackupDelegateCallbacks delegate;
    configureFilesystemDelegate(delegate, open_files, m_cancelled);

    delegate.on_progress = [this](uint64_t, uint64_t, double overall) {
        // overall is device-reported progress (0-100 range, sometimes > 100)
        if (overall > 0 && overall <= 100) {
            qDebug("BackupWorker: %.1f%%", overall);
            emit progress(overall / 100.0);
        } else if (overall > 100) {
            qDebug("BackupWorker: clamping %.1f%% to 100%%", overall);
            emit progress(1.0);
        }
    };

    // Run backup (blocks until complete). Match idevicebackup2 --full by
    // requesting a full backup from the device.
    plist_t options = plist_new_dict();
    plist_dict_set_item(options, "ForceFullBackup", plist_new_bool(1));

    qDebug("BackupWorker: Starting backup...");
    auto result = backup_client.backup(
        backup_root,
        IdeviceFFI::Option<std::string>(udid.toStdString()),
        IdeviceFFI::Option<plist_t>(options),
        delegate);
    plist_free(options);

    qDebug("BackupWorker: backup() returned, is_err=%d", result.is_err());

    // Clean up any remaining open files
    for (auto &[path, stream] : open_files)
        stream.close();

    backup_client.disconnect();

    if (result.is_err()) {
        if (m_cancelled) {
            qDebug("BackupWorker: Cancelled");
            removeCanonicalBackup(backupPath, udid);
            emit cancelled();
            return;
        }

        auto &err = result.unwrap_err();
        qDebug("BackupWorker: Backup failed: code=%d msg=%s", err.code, err.message.c_str());
        emit failed(withEncryptionNote(QString::fromStdString(err.message)));
        return;
    }

    // Check response plist for errors
    auto response = result.unwrap();
    if (!response) {
        const BackupValidationResult validation = validateBackup();
        if (!validation.readable) {
            qDebug("BackupWorker: Backup incomplete: %s", qPrintable(validation.error));
            emit failed(withEncryptionNote(QStringLiteral("Backup incomplete: %1").arg(validation.error)));
            return;
        }
        qDebug("BackupWorker: Backup completed successfully");
        emit finished();
        return;
    }

    uint64_t err_code = 0;
    plist_t err_code_node = plist_dict_get_item(response, "ErrorCode");
    if (err_code_node)
        plist_get_uint_val(err_code_node, &err_code);

    char *err_desc = nullptr;
    plist_t err_desc_node = plist_dict_get_item(response, "ErrorDescription");
    if (err_desc_node)
        plist_get_string_val(err_desc_node, &err_desc);

    qDebug("BackupWorker: Response error code=%llu desc=%s",
           (unsigned long long)err_code, err_desc ? err_desc : "(none)");

    const QString description = err_desc ? QString::fromUtf8(err_desc) : QString();
    free(err_desc);
    plist_free(response);

    if (err_code == 0) {
        const BackupValidationResult validation = validateBackup();
        if (!validation.readable) {
            qDebug("BackupWorker: Backup incomplete: %s", qPrintable(validation.error));
            emit failed(withEncryptionNote(QStringLiteral("Backup incomplete: %1").arg(validation.error)));
            return;
        }
        qDebug("BackupWorker: Backup completed successfully");
        emit finished();
        return;
    }

    if (err_code == 104) {
        const BackupValidationResult validation = validateBackup();
        if (validation.readable) {
            qDebug("BackupWorker: Completed with warnings (error 104 - some files skipped)");
            emit finishedWithWarnings(QStringLiteral("Some files were skipped by the device during backup."));
            return;
        }
        qDebug("BackupWorker: Backup incomplete after error 104: %s", qPrintable(validation.error));
        emit failed(withEncryptionNote(QStringLiteral("Backup incomplete after MobileBackup2 error 104: %1")
                                        .arg(validation.error)));
        return;
    }

    if (err_code == 205 && validateBackup().readable) {
        qDebug("BackupWorker: MobileBackup2 reported error 205, but backup metadata validates");
        emit finishedWithWarnings(QStringLiteral("MobileBackup2 reported a consistency warning, but backup metadata validates."));
        return;
    }

    emit failed(withEncryptionNote(mobileBackupErrorMessage(err_code, description)));
}
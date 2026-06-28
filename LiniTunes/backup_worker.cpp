#include "backup_worker.h"
#include "backup_validator.h"
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
#include <sys/stat.h>
#include <sys/statvfs.h>

namespace {

QString plistDictString(plist_t dict, const char *key)
{
    if (!dict || plist_get_node_type(dict) != PLIST_DICT)
        return {};

    plist_t node = plist_dict_get_item(dict, key);
    if (!node || plist_get_node_type(node) != PLIST_STRING)
        return {};

    char *value = nullptr;
    plist_get_string_val(node, &value);
    const QString result = value ? QString::fromUtf8(value) : QString();
    free(value);
    return result;
}

void plistDictSetString(plist_t dict, const char *key, const QString &value)
{
    if (!value.isEmpty())
        plist_dict_set_item(dict, key, plist_new_string(value.toUtf8().constData()));
}

void plistDictSetCurrentDate(plist_t dict, const char *key)
{
    plist_dict_set_item(dict, key, plist_new_unix_date(QDateTime::currentSecsSinceEpoch()));
}

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

QString mobileBackupErrorMessage(uint64_t code, const QString &description)
{
    if (code == 105) {
        return QStringLiteral("Not enough free disk space to complete this backup. Free up space on the backup drive or choose another backup folder.");
    }

    if (!description.isEmpty())
        return description;

    return QStringLiteral("Backup error code %1").arg(code);
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
    const QString deviceName = plistDictString(values, "DeviceName");
    const QString productType = plistDictString(values, "ProductType");
    const QString productVersion = plistDictString(values, "ProductVersion");
    const QString buildVersion = plistDictString(values, "BuildVersion");
    const QString serialNumber = plistDictString(values, "SerialNumber");
    const QString imei = plistDictString(values, "InternationalMobileEquipmentIdentity");
    const QString meid = plistDictString(values, "MobileEquipmentIdentifier");

    const QString displayName = deviceName.isEmpty() ? udid : deviceName;
    const QString guid = QUuid::createUuid()
                             .toString(QUuid::WithoutBraces)
                             .remove('-')
                             .toUpper();

    plist_t info = plist_new_dict();
    plistDictSetString(info, "Build Version", buildVersion);
    plistDictSetString(info, "Device Name", displayName);
    plistDictSetString(info, "Display Name", displayName);
    plist_dict_set_item(info, "GUID", plist_new_string(guid.toUtf8().constData()));
    plistDictSetString(info, "IMEI", imei);
    plistDictSetString(info, "MEID", meid.isEmpty() && imei.size() >= 14 ? imei.left(14) : meid);
    plistDictSetString(info, "Product Type", productType);
    plistDictSetString(info, "Product Version", productVersion);
    plistDictSetString(info, "Serial Number", serialNumber);
    plist_dict_set_item(info, "Target Identifier", plist_new_string(udid.toUtf8().constData()));
    plist_dict_set_item(info, "Target Type", plist_new_string("Device"));
    plist_dict_set_item(info, "Unique Identifier", plist_new_string(udid.toUtf8().constData()));
    plist_dict_set_item(info, "iTunes Version", plist_new_string("11.2.0"));
    plist_dict_set_item(info, "Installed Applications", plist_new_array());
    plist_dict_set_item(info, "Applications", plist_new_dict());
    plist_dict_set_item(info, "iTunes Files", plist_new_dict());
    plist_dict_set_item(info, "iTunes Settings", plist_new_dict());
    plistDictSetCurrentDate(info, "Last Backup Date");

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
}

BackupWorker::BackupWorker(QObject *parent)
    : QObject(parent) {}

void BackupWorker::cancel()
{
    m_cancelled = true;
    if (m_process && m_process->state() != QProcess::NotRunning)
        m_process->terminate();
}

void BackupWorker::runBackup(const QString &udid, uint32_t deviceId, const QString &backupPath)
{
    m_cancelled = false;

    // Prefer the in-process third_party/idevice MobileBackup2 implementation by
    // default. The external libimobiledevice CLI remains available as an explicit
    // compatibility escape hatch while the direct backend continues to mature.
    const QString requestedBackend = qEnvironmentVariable("LINITUNES_BACKUP_BACKEND").toLower();
    if (requestedBackend == QStringLiteral("idevicebackup2")
        || requestedBackend == QStringLiteral("libimobiledevice")
        || requestedBackend == QStringLiteral("cli")) {
        if (runIdevicebackup2(udid, backupPath))
            return;

        emit failed(QStringLiteral("idevicebackup2 was not found. Install libimobiledevice tools and try again.\nFedora: sudo dnf install libimobiledevice-utils\nmacOS: brew install libimobiledevice"));
        return;
    }

    qDebug("BackupWorker: using direct third_party/idevice MobileBackup2 backend");
    runDirectMobileBackup2(udid, deviceId, backupPath);
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
                emit progress(0, 0, percent / 100.0);
            }
        }
        if (m_cancelled && process.state() != QProcess::NotRunning)
            process.terminate();
    }
    output += QString::fromLocal8Bit(process.readAll());
    m_process = nullptr;

    if (m_cancelled)
        return true;

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

void BackupWorker::runDirectMobileBackup2(const QString &udid, uint32_t deviceId, const QString &backupPath)
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

    // Write host-side Info.plist metadata up front. MobileBackup2 itself writes
    // Status/Manifest data, but Info.plist is host-generated from lockdown
    // values and is used by Finder/iTunes/libimobiledevice for display and
    // compatibility.
    generateInfoPlist(provider, udid, backupPath);

    // Connect to mobilebackup2 service
    auto backup_result = IdeviceFFI::MobileBackup2::connect(provider);
    if (backup_result.is_err()) {
        auto &err = backup_result.unwrap_err();
        qDebug("BackupWorker: Failed to connect to mobilebackup2: %s", err.message.c_str());
        emit failed("Failed to connect to mobilebackup2 service: " +
                     QString::fromStdString(err.message));
        return;
    }
    auto backup_client = std::move(backup_result.unwrap());
    qDebug("BackupWorker: MobileBackup2 connected");

    // Start session (trust/computer authorization)
    auto pf_result = provider.get_pairing_file();
    if (pf_result.is_ok()) {
        qDebug("BackupWorker: Pairing file obtained");
    } else {
        qDebug("BackupWorker: WARNING - no pairing file, trust may fail");
    }

    // Ensure backup directory exists. The backup library stores data under
    // <backup_root>/<udid>/. If a previous attempt left empty metadata plists,
    // iOS may try to read them as an existing backup and fail with
    // MBErrorDomain/205 (zero-length Status.plist). Remove only clearly
    // incomplete metadata so a fresh backup can start cleanly.
    QDir().mkpath(backupPath);
    const QString deviceBackupDir = QDir(backupPath).filePath(udid);
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

    auto validateBackup = [&backupPath, &udid]() {
        return BackupValidator::validate(backupPath, udid);
    };

    std::string backup_root = backupPath.toStdString();
    qDebug("BackupWorker: Backup directory: %s", backup_root.c_str());

    // Track open write files
    std::map<std::string, std::ofstream> open_files;

    // Set up filesystem delegate callbacks
    IdeviceFFI::BackupDelegateCallbacks delegate;

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

    delegate.create_file_write = [&open_files](const std::string &path) {
        open_files[path] = std::ofstream(path, std::ios::binary | std::ios::trunc);
    };

    delegate.write_chunk = [&open_files](const std::string &path, const uint8_t *data, size_t len) {
        auto it = open_files.find(path);
        if (it != open_files.end())
            it->second.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(len));
    };

    delegate.close_file = [&open_files](const std::string &path) {
        auto it = open_files.find(path);
        if (it != open_files.end()) {
            it->second.close();
            open_files.erase(it);
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

    delegate.on_progress = [this](uint64_t bytes_done, uint64_t bytes_total, double overall) {
        // overall is device-reported progress (0-100 range, sometimes > 100)
        if (overall > 0 && overall <= 100) {
            qDebug("BackupWorker: %.1f%%", overall);
            emit progress(0, 0, overall / 100.0);
        } else if (overall > 100) {
            qDebug("BackupWorker: clamping %.1f%% to 100%%", overall);
            emit progress(0, 0, 1.0);
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
    for (auto &[path, stream] : open_files) {
        stream.close();
    }

    if (m_cancelled) {
        qDebug("BackupWorker: Cancelled");
        emit cancelled();
        return;
    }

    if (result.is_err()) {
        auto &err = result.unwrap_err();
        qDebug("BackupWorker: Backup failed: code=%d msg=%s", err.code, err.message.c_str());
        emit failed(QString::fromStdString(err.message));
        return;
    }

    // Check response plist for errors
    auto response = result.unwrap();
    if (response) {
        plist_t err_code_node = plist_dict_get_item(response, "ErrorCode");
        if (err_code_node) {
            uint64_t err_code = 0;
            plist_get_uint_val(err_code_node, &err_code);

            char *err_desc = nullptr;
            plist_t err_desc_node = plist_dict_get_item(response, "ErrorDescription");
            if (err_desc_node)
                plist_get_string_val(err_desc_node, &err_desc);

            qDebug("BackupWorker: Response error code=%llu desc=%s",
                   (unsigned long long)err_code, err_desc ? err_desc : "(none)");

            if (err_code == 0) {
                if (err_desc)
                    free(err_desc);
            } else if (err_code == 104) {
                const BackupValidationResult validation = validateBackup();
                if (validation.readable) {
                    qDebug("BackupWorker: Completed with warnings (error 104 - some files skipped)");
                    if (err_desc)
                        free(err_desc);
                    plist_free(response);
                    backup_client.disconnect();
                    emit finishedWithWarnings(QStringLiteral("Some files were skipped by the device during backup."));
                    return;
                }

                qDebug("BackupWorker: Backup incomplete after error 104: %s", qPrintable(validation.error));
                const QString msg = QStringLiteral("Backup incomplete after MobileBackup2 error 104: %1")
                                        .arg(validation.error);
                if (err_desc)
                    free(err_desc);
                plist_free(response);
                backup_client.disconnect();
                emit failed(msg);
                return;
            } else if (err_code == 205 && validateBackup().readable) {
                qDebug("BackupWorker: MobileBackup2 reported error 205, but backup metadata validates");
                if (err_desc)
                    free(err_desc);
                plist_free(response);
                backup_client.disconnect();
                emit finishedWithWarnings(QStringLiteral("MobileBackup2 reported a consistency warning, but backup metadata validates."));
                return;
            } else {
                const QString description = err_desc ? QString::fromUtf8(err_desc) : QString();
                const QString msg = mobileBackupErrorMessage(err_code, description);
                if (err_desc)
                    free(err_desc);
                plist_free(response);
                backup_client.disconnect();
                emit failed(msg);
                return;
            }
        }

        plist_free(response);
    }

    const BackupValidationResult validation = validateBackup();
    if (!validation.readable) {
        qDebug("BackupWorker: Backup incomplete: %s", qPrintable(validation.error));
        backup_client.disconnect();
        emit failed(QStringLiteral("Backup incomplete: %1").arg(validation.error));
        return;
    }

    backup_client.disconnect();
    qDebug("BackupWorker: Backup completed successfully");
    emit finished();
}
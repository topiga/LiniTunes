#include "backup_worker.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <idevice++/mobilebackup2.hpp>
#include <idevice++/provider.hpp>
#include <idevice++/usbmuxd.hpp>
#include <fstream>
#include <map>
#include <sys/stat.h>
#include <sys/statvfs.h>

BackupWorker::BackupWorker(QObject *parent)
    : QObject(parent) {}

void BackupWorker::cancel()
{
    m_cancelled = true;
}

void BackupWorker::runBackup(const QString &udid, uint32_t deviceId, const QString &backupPath)
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

    // Ensure backup directory exists
    std::string backup_root = backupPath.toStdString();
    QDir().mkpath(backupPath);
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
        if (!f) return {};
        auto size = f.tellg();
        f.seekg(0);
        std::vector<uint8_t> buf(static_cast<size_t>(size));
        f.read(reinterpret_cast<char*>(buf.data()), size);
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

    // Run backup (blocks until complete)
    qDebug("BackupWorker: Starting backup...");
    auto result = backup_client.backup(
        backup_root,
        IdeviceFFI::Option<std::string>(udid.toStdString()),
        IdeviceFFI::Option<plist_t>(),
        delegate);

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

            // Error 104 = multiple file upload errors (non-fatal, normal for unencrypted backups)
            if (err_code == 104) {
                qDebug("BackupWorker: Completed with warnings (error 104 - some files skipped)");
                plist_free(response);
                backup_client.disconnect();
                emit finished();
                return;
            }

            // Other errors are fatal
            QString msg = err_desc ? QString::fromUtf8(err_desc)
                                   : QString("Backup error code %1").arg(err_code);
            plist_free(response);
            backup_client.disconnect();
            emit failed(msg);
            return;
        }

        plist_free(response);
    }

    backup_client.disconnect();
    qDebug("BackupWorker: Backup completed successfully");
    emit finished();
}
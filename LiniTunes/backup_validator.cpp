#include "backup_validator.h"
#include "plist_helpers.h"

#include <QDir>
#include <QFileInfo>
#include <idevice++/bindings.hpp>
#include <cstdlib>

using plist_helpers::stringVal;

namespace {

QString deviceBackupPath(const QString &backupRoot, const QString &udid, const QString &name)
{
    return QDir(QDir(backupRoot).filePath(udid)).filePath(name);
}

bool existsAndNonEmpty(const QString &path)
{
    const QFileInfo fi(path);
    return fi.exists() && fi.isFile() && fi.size() > 0;
}

}

BackupValidationResult BackupValidator::validate(const QString &backupRoot, const QString &udid)
{
    BackupValidationResult result;

    const QStringList requiredFiles = {
        QStringLiteral("Status.plist"),
        QStringLiteral("Manifest.plist"),
        QStringLiteral("Manifest.db")
    };

    QStringList missingOrEmptyFiles;
    for (const QString &requiredFile : requiredFiles) {
        if (!existsAndNonEmpty(deviceBackupPath(backupRoot, udid, requiredFile)))
            missingOrEmptyFiles << requiredFile;
    }

    if (!missingOrEmptyFiles.isEmpty()) {
        result.error = QStringLiteral("Missing or empty backup metadata: %1")
                           .arg(missingOrEmptyFiles.join(QStringLiteral(", ")));
        return result;
    }

    plist_t statusPlist = nullptr;
    const QByteArray statusPath = QFileInfo(deviceBackupPath(backupRoot, udid, QStringLiteral("Status.plist")))
                                      .absoluteFilePath()
                                      .toUtf8();
    if (plist_read_from_file(statusPath.constData(), &statusPlist, nullptr) == PLIST_ERR_SUCCESS && statusPlist) {
        const QString snapshotState = stringVal(statusPlist, "SnapshotState");
        plist_free(statusPlist);
        if (!snapshotState.isEmpty() && snapshotState != QStringLiteral("finished")) {
            result.error = QStringLiteral("Backup snapshot is not finished: %1").arg(snapshotState);
            return result;
        }
    }

    result.readable = true;
    return result;
}

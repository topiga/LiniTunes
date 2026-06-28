#include "backup_validator.h"

#include <QDir>
#include <QFileInfo>
#include <idevice++/bindings.hpp>
#include <cstdlib>

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

QString plistStringValue(plist_t dict, const char *key)
{
    plist_t node = plist_dict_get_item(dict, key);
    if (!node)
        return {};

    char *value = nullptr;
    plist_get_string_val(node, &value);
    const QString result = value ? QString::fromUtf8(value) : QString();
    free(value);
    return result;
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

    for (const QString &requiredFile : requiredFiles) {
        if (!existsAndNonEmpty(deviceBackupPath(backupRoot, udid, requiredFile)))
            result.missingOrEmptyFiles << requiredFile;
    }

    if (!result.missingOrEmptyFiles.isEmpty()) {
        result.error = QStringLiteral("Missing or empty backup metadata: %1")
                           .arg(result.missingOrEmptyFiles.join(QStringLiteral(", ")));
        return result;
    }

    plist_t statusPlist = nullptr;
    const QByteArray statusPath = QFileInfo(deviceBackupPath(backupRoot, udid, QStringLiteral("Status.plist")))
                                      .absoluteFilePath()
                                      .toUtf8();
    if (plist_read_from_file(statusPath.constData(), &statusPlist, nullptr) == PLIST_ERR_SUCCESS && statusPlist) {
        result.snapshotState = plistStringValue(statusPlist, "SnapshotState");
        result.statusParsed = !result.snapshotState.isEmpty();
        result.snapshotFinished = result.snapshotState == QStringLiteral("finished");
        plist_free(statusPlist);
    }

    if (result.statusParsed && !result.snapshotFinished) {
        result.error = QStringLiteral("Backup snapshot is not finished: %1").arg(result.snapshotState);
        return result;
    }

    result.readable = true;
    return result;
}

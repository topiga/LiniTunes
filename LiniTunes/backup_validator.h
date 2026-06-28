#ifndef BACKUP_VALIDATOR_H
#define BACKUP_VALIDATOR_H

#include <QString>
#include <QStringList>

struct BackupValidationResult
{
    bool readable = false;
    bool statusParsed = false;
    bool snapshotFinished = false;
    QString snapshotState;
    QStringList missingOrEmptyFiles;
    QString error;
};

class BackupValidator
{
public:
    static BackupValidationResult validate(const QString &backupRoot, const QString &udid);
};

#endif // BACKUP_VALIDATOR_H

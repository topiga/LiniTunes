#ifndef BACKUP_VALIDATOR_H
#define BACKUP_VALIDATOR_H

#include <QString>

struct BackupValidationResult
{
    bool readable = false;
    QString error;
};

class BackupValidator
{
public:
    static BackupValidationResult validate(const QString &backupRoot, const QString &udid);
};

#endif // BACKUP_VALIDATOR_H

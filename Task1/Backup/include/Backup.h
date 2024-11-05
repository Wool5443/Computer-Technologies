#ifndef BACKUP_H
#define BACKUP_H

#include "Error.h"

#define MAX_PATH_SIZE 4096

typedef struct
{
    char* backupPath;
    char* storagePath;
} Backupper;

DECLARE_RESULT(Backupper);

ResultBackupper BackupperCtor(const char backupFolder[static 1], const char storageFolder[static 1]);
void            BackupperDtor(Backupper* backupper);
ErrorCode       BackupperVerify(const Backupper* backupper);

ErrorCode       Backup(Backupper backupper[static 1]);

#endif

#ifndef BACKUP_H
#define BACKUP_H

#include "Error.h"

typedef struct
{
    const char* backupFolder;
    const char* storageFolder;
} Backupper;

DECLARE_RESULT(Backupper);

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder);

#endif

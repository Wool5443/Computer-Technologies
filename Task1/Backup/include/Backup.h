#ifndef BACKUP_H

#define BACKUP_H

#include "FileList.h"

#define MAX_PATH_SIZE 4096

typedef struct
{
    const char* backupPath;
    const char* storagePath;
    FileList    saveFileList;
    FileList    storageFileList;
} Backupper;

DECLARE_RESULT(Backupper);

ResultBackupper BackupperCtor(const char backupFolder[static 1], const char storageFolder[static 1]);
void            BackupperDtor(Backupper* backupper);
ErrorCode       BackupperVerify(const Backupper* backupper);

ErrorCode       Backup(const Backupper backupper[static 1]);

#endif

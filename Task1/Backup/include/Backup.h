#ifndef BACKUP_H
#define BACKUP_H

#include <dirent.h>
#include "Error.h"

typedef struct
{
    DIR* backupDir;
    DIR* storageDir;
} Backupper;

typedef struct
{
    size_t       size;
    const char** files;
} FileList;

DECLARE_RESULT(Backupper);
DECLARE_RESULT(FileList);

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder);
void            BackupperDtor(Backupper* backupper);
ErrorCode       BackupperVerify(const Backupper* backupper);

ResultFileList  FileListCtor(const Backupper* backupper);
void            FileListDtor(FileList* list);

#endif

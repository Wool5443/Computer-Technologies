#ifndef BACKUP_H
#define BACKUP_H

#include <dirent.h>
#include "Error.h"

#define MAX_PATH_SIZE 4096

typedef char** FileList;

typedef struct
{
    const char* backupPath;
    const char* storagePath;
    FileList    fileList;
} Backupper;

DECLARE_RESULT(Backupper);

typedef struct
{
    ErrorCode error;
    FileList value;
} ResultFileList;

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder);
void            BackupperDtor(Backupper* backupper);
ErrorCode       BackupperVerify(const Backupper* backupper);

ResultFileList  FileListCtor(const char* dir);
void            FileListDtor(FileList list);

#endif

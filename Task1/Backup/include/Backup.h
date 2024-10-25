#ifndef BACKUP_H

#define BACKUP_H

#include <time.h>
#include "Error.h"

#define MAX_PATH_SIZE 4096

typedef struct
{
    char* path;
    time_t updateDate;
} FileEntry;

typedef FileEntry* FileList;

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

ResultBackupper BackupperCtor(const char backupFolder[static 1], const char storageFolder[static 1]);
void            BackupperDtor(Backupper* backupper);
ErrorCode       BackupperVerify(const Backupper* backupper);

ResultFileList  FileListCtor(const char dir[static 1]);
void            FileListDtor(FileList list);

#endif

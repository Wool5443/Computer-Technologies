#ifndef BACKUP_H
#define BACKUP_H

#include "Error.h"
#include "StringSlice.h"

#define MAX_PATH_SIZE 4096

typedef struct
{
    char*  data;
    size_t size;
} String;

String    SanitizeDirectoryPath(const char path[static 1]);

ErrorCode Backup(StringSlice backupPath, StringSlice storagePath);
ErrorCode Restore(StringSlice storagePath);

#endif

#ifndef BACKUP_H
#define BACKUP_H

#include "String.h"

#define MAX_PATH_SIZE 4096

ResultString SanitizeDirectoryPath(const char path[static 1]);

ErrorCode Backup(Str backupPath, Str storagePath);
ErrorCode Restore(Str storagePath);

#endif

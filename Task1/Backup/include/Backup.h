#ifndef BACKUP_H
#define BACKUP_H

#include "Error.h"

#define MAX_PATH_SIZE 4096

char*     SanitizePath(const char path[static 1]);

ErrorCode Backup(const char backupPath[static 1], const char storagePath[static 1]);
ErrorCode Restore(const char storagePath[static 1]);

#endif

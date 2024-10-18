#include <assert.h>
#include <stdio.h>
#include <dirent.h>
#include "Backup.h"

static void safeclosedir(DIR* dir);

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder)
{
    ERROR_CHECKING();
    ResultBackupper result = {};

    assert(backupFolder);
    assert(storageFolder);

    DIR* backdir = opendir(backupFolder);
    DIR* storagedir = opendir(storageFolder);

    if (!backdir || !storagedir)
    {
        err = ERROR_BAD_FOLDER;
        goto cleanup;
    }

    struct dirent* entry = NULL;
    while ((entry = readdir(backdir)))
    {
        printf("Found entry %s\n", entry->d_name);
    }

    cleanup:
    safeclosedir(backdir);
    safeclosedir(storagedir);
    if (err)
        RETURN((ResultBackupper){}, err);

    return result;
}

static void safeclosedir(DIR* dir)
{
    if (dir)
        closedir(dir);
}

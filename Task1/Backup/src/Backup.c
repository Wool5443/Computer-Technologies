#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "ScratchBuf.h"
#include "Backup.h"

#define BAD_COUNT (size_t)-1
#define MAX_PATH_SIZE 1024

static void safeclosedir(DIR* dir);
size_t countFiles(DIR* dir, const char** curFileSpot);

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder)
{
    ERROR_CHECKING();

    assert(backupFolder);
    assert(storageFolder);

    DIR* backdir = opendir(backupFolder);
    DIR* storagedir = opendir(storageFolder);

    if (!backdir || !storagedir)
    {
        err = ERROR_BAD_FOLDER;
        LOG_IF_ERROR();
        goto errorReturn;
    }

    return (ResultBackupper) {
        err,
        { .backupDir = backdir, .storageDir = storagedir },
    };

errorReturn:
    safeclosedir(backdir);
    safeclosedir(storagedir);
    return (ResultBackupper){ err, {} };
}

void BackupperDtor(Backupper* backupper)
{
    assert(backupper);

    safeclosedir(backupper->backupDir);
    safeclosedir(backupper->storageDir);
}

ErrorCode BackupperVerify(const Backupper* backupper)
{
    if (!backupper)
        return ERROR_NULLPTR;

    if (!backupper->backupDir || !backupper->storageDir)
        return ERROR_BAD_FILE;

    return EVERYTHING_FINE;
}

ResultFileList FileListCtor(const Backupper* backupper)
{
    assert(backupper);
    ERROR_CHECKING();

    if ((err = BackupperVerify(backupper)))
    {
        LOG_IF_ERROR();
        goto errorRet;
    }

    DIR* backdir = backupper->backupDir;

    size_t size = countFiles(backdir, NULL);
    if (size == BAD_COUNT)
    {
        err = ERROR_BAD_FOLDER;
        goto errorRet;
    }

    const char** files = calloc(size, sizeof(*files));
    if (!files)
    {
        err = ERROR_NO_MEMORY;
        goto errorRet;
    }

    countFiles(backdir, files);

    return (ResultFileList) {
        .error = err,
        .value = (FileList) {
            .size = size,
            .files = files,
        },
    };

errorRet:
    return (ResultFileList){ err, {} };
}

void FileListDtor(FileList* list)
{
    assert(list);
    free(list->files);
}

size_t countFiles(DIR* dir, const char** curFileSpot)
{
    assert(dir);

    ERROR_CHECKING();

    size_t files = 0;

    struct dirent* e = NULL;

    while ((e = readdir(dir)))
    {
        LOG("e name: %s\n", e->d_name);

        if (e->d_type == DT_REG)
        {
            if (curFileSpot)
            {
                *curFileSpot = e->d_name;
                curFileSpot++;
            }
            files++;
        }
        else if (e->d_type == DT_DIR)
        {
            if (e->d_name[0] == '.') continue;
            DIR* subdir = opendir(NULL);
            if (!subdir)
            {
                err = ERROR_BAD_FOLDER;
                LOG_IF_ERROR();
                LOG("Name = %s\n", SCRATCH_PATH);
                perror("OPENDIR ERROR\n");
                return BAD_COUNT;
            }

            size_t cnt = countFiles(subdir, curFileSpot);
            closedir(subdir);

            if (cnt == BAD_COUNT)
                return BAD_COUNT;
        }
    }

    return files;
}

static void safeclosedir(DIR* dir)
{
    if (dir)
        closedir(dir);
}

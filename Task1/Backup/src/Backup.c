#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "FileList.h"
#include "ScratchBuf.h"
#include "Vector.h"
#include "Backup.h"

#define LINUX_ERROR -1
#define BAD_COUNT (size_t)-1
#define BACKUP_TABLE_NAME ".backTable"

static void safeclose(FILE file[static 1]);

static void copyAndZip(const char src[static 1]);

ResultBackupper BackupperCtor(const char backupFolder[static 1], const char storageFolder[static 1])
{
    ERROR_CHECKING();

    assert(backupFolder);
    assert(storageFolder);

    char* backupstr  = NULL;
    char* storagestr = NULL;

    StringSlice backupPath  = StringSliceCtor(backupFolder);
    StringSlice storagePath = StringSliceCtor(storageFolder);

    if (backupPath.data[backupPath.size - 1] != '/')
    {
        ScratchBufClean();
        ScratchAppendSlice(backupPath);
        ScratchAppendChar('/');
        backupstr = strdup(ScratchGetStr());
    }
    else
    {
        backupstr = strdup(backupFolder);
    }

    if (!backupstr)
    {
        err = ERROR_NO_MEMORY;
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    if (storagePath.data[storagePath.size - 1] != '/')
    {
        ScratchBufClean();
        ScratchAppendSlice(storagePath);
        ScratchAppendChar('/');
        storagestr = strdup(ScratchGetStr());
    }
    else
    {
        storagestr = strdup(storageFolder);
    }

    if (!storagestr)
    {
        err = ERROR_NO_MEMORY;
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    return (ResultBackupper) {
        err,
        {
            .backupPath  = backupstr,
            .storagePath = storagestr,
        },
    };

ERROR_CASE
    free(backupstr);
    free(storagestr);
    return (ResultBackupper){ err, {} };
}

void BackupperDtor(Backupper* backupper)
{
    if (!backupper) return;

    free(backupper->backupPath);
    free(backupper->storagePath);
}

ErrorCode BackupperVerify(const Backupper* backupper)
{
    if (!backupper)
        return ERROR_NULLPTR;

    if (!backupper->backupPath || !backupper->storagePath)
        return ERROR_BAD_FILE;

    return EVERYTHING_FINE;
}

ErrorCode Backup(Backupper backupper[static 1])
{
    ERROR_CHECKING();

    assert(backupper);

    FileList filesToSaveList = NULL;

    ResultFileList filesToSaveListRes = FileListCtor(backupper->backupPath);

    if ((err = filesToSaveListRes.error))
    {
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    filesToSaveList = filesToSaveListRes.value;

    LOG("size = %zu\n", VecSize(filesToSaveList));
    for (size_t i = 0, sz = VecSize(filesToSaveList); i < sz; i++)
    {
        FileEntry ent = filesToSaveList [i];
        printf("%s: %ld\n", ent.path, ent.updateDate);
    }

    for (size_t i = 0, end = VecSize(filesToSaveList); i < end; i++)
    {
        FileEntry  saveEntry    = filesToSaveList[i];
        FileEntry* storageEntry = NULL;

        ScratchBufClean();
        ScratchAppendStr(backupper->storagePath);

        char* filename = ScratchGetStr() + ScratchGetSize();

        ScratchAppendStr(saveEntry.path);

        char* dirchar = strchr(filename, '/');

        while (dirchar)
        {
            *dirchar = '.';
            dirchar = strchr(dirchar + 1, '/');
        }

        ScratchAppendStr(".tar.gz");
        LOG("dest = %s\n", ScratchGetStr());

        copyAndZip(saveEntry.path);

        if (storageEntry)
            storageEntry->updateDate = saveEntry.updateDate;
        else
            VecAdd(filesToSaveList, saveEntry);
    }

    return EVERYTHING_FINE;

ERROR_CASE
    VecDtor(filesToSaveList);
    return err;
}

[[maybe_unused]] void safeclose(FILE file[static 1])
{
    if (file)
        fclose(file);
}

static void copyAndZip(const char src[static 1])
{
    ERROR_CHECKING();

    assert(src);

    {
        struct stat archstat = {};
        if (stat(ScratchGetStr(), &archstat) == 0)
        {
            struct stat filestat = {};
            stat(src, &filestat);
            if (archstat.st_mtim.tv_sec == filestat.st_mtim.tv_sec)
                return;
        }
    }

    pid_t copypid = fork();

    if (copypid == 0)
    {
        pid_t touchpid = fork();
        if (touchpid == 0)
        {
            const char* args[] = { "touch", src, NULL };
            execvp("touch", (char**)args);
        }
        else if (touchpid == LINUX_ERROR)
        {
            err = ERROR_LINUX;
            LOG_IF_ERROR();
        }
        // child
        char* lastdirsep = strrchr(src, '/');
        *lastdirsep = '\0';
        const char* args[] = { "tar", "-czf", ScratchGetStr(), "-C", src, lastdirsep + 1, NULL };
        execvp("tar", (char**)args);
    }
    else if (copypid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LOG_IF_ERROR();
    }
}

#include <stdlib.h>
#include <unistd.h>

#include "ScratchBuf.h"
#include "Vector.h"
#include "Backup.h"

#define LINUX_ERROR -1
#define BAD_COUNT (size_t)-1
#define BACKUP_TABLE_NAME ".backTable"

static ResultFileList readBackupTable (const char backupTablePath[static 1]);
static void           writeBackupTable(const char backupTablePath[static 1], FileList fileList);
static void           safeclose(FILE* file);

static void           copyAndZip(const char dest[static 1], const char src[static 1]);

ResultBackupper BackupperCtor(const char backupFolder[static 1], const char storageFolder[static 1])
{
    ERROR_CHECKING();

    assert(backupFolder);
    assert(storageFolder);

    FileList saveFileList    = NULL;
    FileList storageFileList = NULL;

    ResultFileList saveFileListRes = FileListCtor(backupFolder);

    if ((err = saveFileListRes.error))
    {
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }
    saveFileList = saveFileListRes.value;

    ScratchBufClean();
    ScratchAppendStr(storageFolder);
    if (ScratchGetStr()[ScratchGetSize() - 1] != '/')
        ScratchAppendChar('/');
    ScratchAppendStr(BACKUP_TABLE_NAME);

    ResultFileList storageFileListRes = readBackupTable(ScratchGetStr());
    switch (storageFileListRes.error)
    {
        case ERROR_NOT_FOUND:
            break;
        case EVERYTHING_FINE:
            storageFileList = storageFileListRes.value;
        default:
            err = storageFileListRes.error;
            LOG_IF_ERROR();
            ERROR_LEAVE();
    }

    return (ResultBackupper) {
        err,
        {
            .backupPath      = backupFolder,
            .storagePath     = storageFolder,
            .saveFileList    = saveFileList,
            .storageFileList = storageFileList,
        },
    };

ERROR_CASE
    FileListDtor(saveFileList);
    FileListDtor(storageFileList);
    return (ResultBackupper){ err, {} };
}

void BackupperDtor(Backupper* backupper)
{
    if (!backupper) return;

    FileListDtor(backupper->saveFileList);
}

ErrorCode BackupperVerify(const Backupper* backupper)
{
    if (!backupper)
        return ERROR_NULLPTR;

    if (!backupper->backupPath || !backupper->storagePath)
        return ERROR_BAD_FILE;

    return EVERYTHING_FINE;
}

ErrorCode Backup(const Backupper backupper[static 1])
{
    assert(backupper);



    return EVERYTHING_FINE;
}

ResultFileList readBackupTable(const char backupTablePath[static 1])
{
    ERROR_CHECKING();

    assert(backupTablePath);

    FILE* backupFile = fopen(backupTablePath, "rb");
    FileList backupList = NULL;

    if (!backupFile)
    {
        err = ERROR_NOT_FOUND;
        ERROR_LEAVE();
    }

    size_t tableSize = 0;
    if (fread(&tableSize, sizeof(tableSize), 1, backupFile) != 0)
    {
        err = ERROR_BAD_FILE;
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    backupList = VecCtor(sizeof(*backupList), tableSize);
    if (!backupList)
    {
        err = ERROR_NO_MEMORY;
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    ScratchBufClean();

    for (size_t i = 0; i < tableSize; i++)
    {
        size_t slen = 0;
        if (fread(&slen, sizeof(slen), 1, backupFile) != 0)
        {
            err = ERROR_BAD_FILE;
            LOG_IF_ERROR();
            ERROR_LEAVE();
        }

        time_t updateDate = 0;
        if (fread(&updateDate, sizeof(updateDate), 1, backupFile) != 0)
        {
            err = ERROR_BAD_FILE;
            LOG_IF_ERROR();
            ERROR_LEAVE();
        }

        if (fread(ScratchGetStr(), 1, slen, backupFile) != 0)
        {
            err = ERROR_BAD_FILE;
            LOG_IF_ERROR();
            ERROR_LEAVE();
        }

        FileEntry entry = { strdup(ScratchGetStr()), updateDate };
        VecAdd(backupList, entry);
    }

    return (ResultFileList) { err, backupList };

ERROR_CASE
    safeclose(backupFile);
    FileListDtor(backupList);
    return (ResultFileList) { err, {} };
}

static void writeBackupTable(const char backupTablePath[static 1], FileList backupTable)
{
    ERROR_CHECKING();

    assert(backupTablePath);
    assert(backupTable);

    FILE* backupTableFile = NULL;

    backupTableFile = fopen(backupTablePath, "wb");

    if (!backupTableFile)
    {
        err = ERROR_BAD_FILE;
        LOG_IF_ERROR();
        ERROR_LEAVE();
    }

    size_t size = VecSize(backupTable);

    fwrite(&size, sizeof(size), 1, backupTableFile);

    for (size_t i = 0; i < size; i++)
    {
        struct LenTime
        {
            size_t len;
            time_t time;
        };

        struct LenTime lt = { strlen(backupTable[i].path), backupTable[i].updateDate };

        fwrite(&lt, sizeof(lt), 1, backupTableFile);
        fwrite(backupTable[i].path, 1, lt.len, backupTableFile);
    }

ERROR_CASE
    safeclose(backupTableFile);
}

void safeclose(FILE* file)
{
    if (file)
        fclose(file);
}

static void copyAndZip(const char dest[static 1], const char src[static 1])
{
    ERROR_CHECKING();

    assert(dest);
    assert(src);

    pid_t copypid = fork();

    if (copypid == 0)
    {
        // child
        const char* args[] = { "-czf", dest, src, NULL };
        execvp("tar", (char**)args);

    }
    else if (copypid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LOG_IF_ERROR();
    }
}

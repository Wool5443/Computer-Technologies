#include <assert.h>
#include <stdlib.h>

#define __USE_XOPEN_EXTENDED
#include <ftw.h>

#include "Vector.h"
#include "Backup.h"

#define MAX_FD 32
#define LINUX_ERROR -1
#define BAD_COUNT (size_t)-1

static void safeclosedir(DIR* dir);

ResultBackupper BackupperCtor(const char* backupFolder, const char* storageFolder)
{
    ERROR_CHECKING();

    assert(backupFolder);
    assert(storageFolder);

    ResultFileList fileListRes = FileListCtor(backupFolder);

    if ((err = fileListRes.error))
    {
        LOG_IF_ERROR();
        goto errorReturn;
    }

    return (ResultBackupper) {
        err,
        {
            .backupPath  = backupFolder,
            .storagePath = storageFolder,
            .fileList = fileListRes.value,
        },
    };

errorReturn:
    return (ResultBackupper){ err, {} };
}

void BackupperDtor(Backupper* backupper)
{
    assert(backupper);

    FileListDtor(backupper->fileList);
}

ErrorCode BackupperVerify(const Backupper* backupper)
{
    if (!backupper)
        return ERROR_NULLPTR;

    if (!backupper->backupPath || !backupper->storagePath)
        return ERROR_BAD_FILE;

    return EVERYTHING_FINE;
}

int fileListFn(const char *fpath, [[maybe_unused]] const struct stat *sb,
               int typeflag, [[maybe_unused]] struct FTW *ftwbuf);

static FileList fileNames = NULL;
ResultFileList FileListCtor(const char* dir)
{
    ERROR_CHECKING();
    assert(dir);

    if ((err = nftw(dir, fileListFn, MAX_FD, 0)))
    {
        LOG_IF_ERROR();
        goto errorRet;
    }

    return (ResultFileList){ err, fileNames };

errorRet:
    FileListDtor(fileNames);
    return (ResultFileList){ err, {} };
}

void FileListDtor(FileList list)
{
    assert(list);

    for (size_t i = 0, end = VecSize(list); i < end; i++)
        free(list[i]);
    VecDtor(list);
}

int fileListFn(const char *fpath, [[maybe_unused]] const struct stat *sb,
               int typeflag, [[maybe_unused]] struct FTW *ftwbuf)
{
    if (typeflag == FTW_F)
    {
        VecAdd(fileNames, strdup(fpath));
    }
    return 0;
}

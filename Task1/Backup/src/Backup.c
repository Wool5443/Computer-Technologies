#include <assert.h>
#include <stdlib.h>

#define __USE_XOPEN_EXTENDED
#include <ftw.h>

#include "Vector.h"
#include "Backup.h"

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

    FileListDtor(&backupper->fileList);
}

ErrorCode BackupperVerify(const Backupper* backupper)
{
    if (!backupper)
        return ERROR_NULLPTR;

    if (!backupper->backupPath || !backupper->storagePath)
        return ERROR_BAD_FILE;

    return EVERYTHING_FINE;
}

int fileListFn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    static const char** fileNames = NULL;
    if (typeflag == FTW_F)
    {
        VecAdd(fileNames, fpath);
    }
    return 0;
}

ResultFileList FileListCtor(const char* dir)
{
    ERROR_CHECKING();
    assert(dir);

    /* nftw */

errorRet:
    return (ResultFileList){ err, {} };
}

void FileListDtor(FileList* list)
{
    assert(list);
    free(list->files);
}

static void safeclosedir(DIR* dir)
{
    if (dir)
        closedir(dir);
}

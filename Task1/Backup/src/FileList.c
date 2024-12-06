#include <unistd.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>

#include "FileList.h"
#include "Vector.h"

int fileListFn(const char *fpath, [[maybe_unused]] const struct stat *sb,
               int typeflag, [[maybe_unused]] struct FTW *ftwbuf);

static FileList fileNames = NULL;
ResultFileList FileListCtor(const char dir[static 1])
{
    ERROR_CHECKING();

    assert(dir);

    if (nftw(dir, fileListFn, MAX_FD, 0) != 0)
    {
        err = ERROR_BAD_FOLDER;
        LogError();
        ERROR_LEAVE();
    }

    return ResultFileListCtor(fileNames, err);

ERROR_CASE
    FileListDtor(fileNames);
    return ResultFileListCtor((FileList){}, err);
}

void FileListDtor(FileList list)
{
    if (!list) return;

    for (size_t i = 0, end = VecSize(list); i < end; i++)
        free(list[i].path);
    VecDtor(list);
}

time_t FileEntryCompare(const void* a, const void* b)
{
    if (!a || !b)
        return -1;

    FileEntry entA = *(FileEntry*)a;
    FileEntry entB = *(FileEntry*)b;

    int strdif = strcmp(entA.path, entB.path);

    if (strdif != 0) return strdif;

    return entA.updateDate - entB.updateDate;
}

FileEntry* FindFileEntry(const FileList fileList, FileEntry entry)
{
    assert(fileList);

    for (size_t i = 0, end = VecSize(fileList); i < end; i++)
        if (strcmp(fileList[i].path, entry.path))
            return fileList + i;

    return NULL;
}

int fileListFn(const char *fpath, [[maybe_unused]] const struct stat *sb,
               int typeflag, [[maybe_unused]] struct FTW *ftwbuf)
{
    if (typeflag == FTW_F)
    {
        time_t updated = sb->st_mtim.tv_sec;
        FileEntry fent = { strdup(fpath), updated };
        VecAdd(fileNames, fent);
    }
    return 0;
}

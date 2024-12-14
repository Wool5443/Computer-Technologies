#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include "FileList.h"
#include "ScratchBuf.h"
#include "Vector.h"
#include "Backup.h"

#define LINUX_ERROR -1
#define BAD_COUNT (size_t)-1

static const size_t ARCHIVE_SUFFIX_LENGTH = strlen(".tar.gz");

static const char SLASH_REPLACE_SYMBOL = '*';

static void copyAndZip(FileEntry saveFile, Str storagePath);
static void unzip(FileEntry archive);

ErrorCode Backup(Str backupPath, Str storagePath)
{
    ERROR_CHECKING();

    assert(backupPath.data);
    assert(storagePath.data);

    FileList filesToSaveList = NULL;

    ResultFileList filesToSaveListRes = FileListCtor(backupPath.data);
    CHECK_ERROR(filesToSaveListRes.errorCode);

    filesToSaveList = filesToSaveListRes.value;

    for (size_t i = 0, end = VecSize(filesToSaveList); i < end; i++)
        copyAndZip(filesToSaveList[i], storagePath);

    return EVERYTHING_FINE;

ERROR_CASE
    VecDtor(filesToSaveList);
    return err;
}

ErrorCode Restore(Str storagePath)
{
    ERROR_CHECKING();

    assert(storagePath.data);

    FileList fileList = NULL;

    ResultFileList fileListRes = FileListCtor(storagePath.data);
    CHECK_ERROR(fileListRes.errorCode);

    fileList = fileListRes.value;

    for (size_t i = 0, end = VecSize(fileList); i < end; i++)
    {
        unzip(fileList[i]);
    }

    return EVERYTHING_FINE;

ERROR_CASE
    FileListDtor(fileList);

    return err;
}

ResultString SanitizeDirectoryPath(const char path[static 1])
{
    ERROR_CHECKING();

    assert(path);

    char goodPath[PATH_MAX] = "";

    if (!realpath(path, goodPath))
    {
        err = ERROR_BAD_FOLDER;
        LogError();
        return (ResultString){ {}, err };
    }

    size_t size = strlen(goodPath);

    goodPath[size] = '/';
    size++;

    return StringCtorFromStr(StrCtorSize(goodPath, size));
}

static void copyAndZip(FileEntry saveFile, Str storagePath)
{
    ERROR_CHECKING();

    assert(storagePath.data);

    Str saveFileStr = StrCtor(saveFile.path);

    ScratchClean();

    ScratchAppendStr(storagePath);
    size_t fileNameIndex = ScratchGetSize();
    ScratchAppendStr(saveFileStr);
    ScratchAppend(".tar.gz");

    char* slash = strchr(&ScratchGet()[fileNameIndex], '/');

    while (slash)
    {
        *slash = SLASH_REPLACE_SYMBOL;
        slash = strchr(slash + 1, '/');
    }

    {
        struct stat archstat = {};
        if (stat(ScratchGet(), &archstat) == 0)
        {
            struct stat filestat = {};
            stat(saveFileStr.data, &filestat);
            if (archstat.st_mtim.tv_sec >= filestat.st_mtim.tv_sec)
                return;
        }
    }

    char* lastdirsep = strrchr(saveFileStr.data, '/'); // TODO we know length
    *lastdirsep = '\0';

    pid_t copypid = vfork();

    if (copypid == 0)
    {
        printf("%s was backed up!\n", saveFileStr.data);
        fflush(stdout);
        const char* args[] = { "tar", "-czf", ScratchGet(), "-C", saveFileStr.data, lastdirsep + 1, NULL };
        execvp("tar", (char**)args);
    }
    else if (copypid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LogError();
        return;
    }
}

static void unzip(FileEntry archive)
{
    ERROR_CHECKING();

    ScratchClean();
    ScratchAppend(strchr(archive.path, SLASH_REPLACE_SYMBOL));

    // delete .tar.gz at the end
    ScratchGet()[ScratchGetSize() - ARCHIVE_SUFFIX_LENGTH] = '\0';

    char* lastSlash = NULL;
    char* slash = strchr(ScratchGet(), SLASH_REPLACE_SYMBOL);

    while (slash)
    {
        *slash = '/';
        lastSlash = slash;
        slash = strchr(slash + 1, SLASH_REPLACE_SYMBOL);
    }

    {
        struct stat filestat = {};
        if (stat(ScratchGet(), &filestat) == 0)
        {
            struct stat archstat = {};
            stat(archive.path, &archstat);
            if (archstat.st_mtim.tv_sec >= filestat.st_mtim.tv_sec)
                return;
        }
    }

    pid_t mkdirpid = vfork();

    if (mkdirpid == 0)
    {
        *lastSlash = '\0';
        const char* args[] = { "mkdir", "-p", ScratchGet(), NULL };
        execvp("mkdir", (char**)args);
    }
    else if (mkdirpid == LINUX_ERROR)
    {
        err = LINUX_ERROR;
        LogError();
        return;
    }

    pid_t unzipid = vfork();

    if (unzipid == 0)
    {
        *lastSlash = '/';
        printf("%s was restored!\n", ScratchGet());
        fflush(stdout);
        *lastSlash = '\0';
        const char* args[] = { "tar" , "-xzf", archive.path, "-C", ScratchGet(), NULL };
        execvp("tar", (char**)args);
    }
    else if (unzipid == LINUX_ERROR)
    {
        err = LINUX_ERROR;
        LogError();
        return;
    }

    pid_t touchpid = vfork();
    if (touchpid == 0)
    {
        const char* args[] = { "touch", archive.path, NULL };
        execvp("touch", (char**)args);
    }
    else if (touchpid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LogError();
        return;
    }
}

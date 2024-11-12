#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "Error.h"
#include "FileList.h"
#include "ScratchBuf.h"
#include "Vector.h"
#include "Backup.h"

#define LINUX_ERROR -1
#define BAD_COUNT (size_t)-1
#define BACKUP_TABLE_NAME ".backTable"

static const char SLASH_REPLACE_SYMBOL = '*';

static void copyAndZip(FileEntry saveFile, StringSlice storagePath);
static void unzip(FileEntry archive);

ErrorCode Backup(StringSlice backupPath, StringSlice storagePath)
{
    ERROR_CHECKING();

    assert(backupPath.data);
    assert(storagePath.data);

    FileList filesToSaveList = NULL;

    ResultFileList filesToSaveListRes = FileListCtor(backupPath.data);
    CHECK_ERROR(filesToSaveListRes.error);

    filesToSaveList = filesToSaveListRes.value;

    printf("Backing up these files:\n");
    for (size_t i = 0, sz = VecSize(filesToSaveList); i < sz; i++)
    {
        FileEntry ent = filesToSaveList[i];
        printf("%s\n", ent.path);
    }
    printf("\n");

    for (size_t i = 0, end = VecSize(filesToSaveList); i < end; i++)
    {
        copyAndZip(filesToSaveList[i], storagePath);
    }

    return EVERYTHING_FINE;

ERROR_CASE
    VecDtor(filesToSaveList);
    return err;
}

ErrorCode Restore(StringSlice storagePath)
{
    ERROR_CHECKING();

    assert(storagePath.data);

    FileList fileList = NULL;

    ResultFileList fileListRes = FileListCtor(storagePath.data);
    CHECK_ERROR(fileListRes.error);

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

String SanitizeDirectoryPath(const char path[static 1])
{
    assert(path);

    size_t len = strlen(path);

    char* newPath = calloc(len + 2, 1);
    if (!newPath) return (String){};

    memcpy(newPath, path, len);

    if (newPath[len - 1] != '/')
    {
        newPath[len] = '/';
        len++;
    }

    return (String){ newPath, len };
}

static void copyAndZip(FileEntry saveFile, StringSlice storagePath)
{
    ERROR_CHECKING();

    assert(storagePath.data);

    StringSlice saveFileSlice = StringSliceCtor(saveFile.path);

    ScratchBufClean();

    ScratchAppendSlice(storagePath);
    size_t fileNameIndex = ScratchGetSize();
    ScratchAppendSlice(saveFileSlice);
    ScratchAppendStr(".tar.gz");

    char* slash = strchr(&ScratchGetStr()[fileNameIndex], '/');

    while (slash)
    {
        *slash = SLASH_REPLACE_SYMBOL;
        slash = strchr(slash + 1, '/');
    }

    {
        struct stat archstat = {};
        if (stat(ScratchGetStr(), &archstat) == 0)
        {
            struct stat filestat = {};
            stat(saveFileSlice.data, &filestat);
            if (archstat.st_mtim.tv_sec >= filestat.st_mtim.tv_sec)
                return;
        }
    }

    char* lastdirsep = strrchr(saveFileSlice.data, '/');
    *lastdirsep = '\0';

    pid_t copypid = fork();

    if (copypid == 0)
    {
        const char* args[] = { "tar", "-czf", ScratchGetStr(), "-C", saveFileSlice.data, lastdirsep + 1, NULL };
        execvp("tar", (char**)args);
    }
    else if (copypid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LOG_ERROR();
        return;
    }

    pid_t touchpid = fork();
    if (touchpid == 0)
    {
        const char* args[] = { "touch", saveFileSlice.data, NULL };
        execvp("touch", (char**)args);
    }
    else if (touchpid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LOG_ERROR();
        return;
    }
}

static void unzip(FileEntry archive)
{
    ERROR_CHECKING();

    char destinationFolder[MAX_PATH_SIZE] = "";
    strncpy(destinationFolder, strchr(archive.path, SLASH_REPLACE_SYMBOL), MAX_PATH_SIZE);

    char* lastSlash = NULL;
    char* slash = strchr(destinationFolder, SLASH_REPLACE_SYMBOL);

    while (slash)
    {
        *slash = '/';
        lastSlash = slash;
        slash = strchr(slash + 1, SLASH_REPLACE_SYMBOL);
    }

    *(lastSlash + 1) = '\0';

    pid_t mkdirpid = fork();

    if (mkdirpid == 0)
    {
        const char* args[] = { "mkdir", "-p", destinationFolder, NULL };
        execvp("mkdir", (char**)args);
    }
    else if (mkdirpid == LINUX_ERROR)
    {
        err = LINUX_ERROR;
        LOG_ERROR();
        return;
    }

    pid_t unzipid = fork();

    if (unzipid == 0)
    {
        const char* args[] = { "tar" , "-xzf", archive.path, "-C", destinationFolder, NULL };
        execvp("tar", (char**)args);
    }
    else if (unzipid == LINUX_ERROR)
    {
        err = LINUX_ERROR;
        LOG_ERROR();
        return;
    }

    pid_t touchpid = fork();
    if (touchpid == 0)
    {
        const char* args[] = { "touch", archive.path, NULL };
        execvp("touch", (char**)args);
    }
    else if (touchpid == LINUX_ERROR)
    {
        err = ERROR_LINUX;
        LOG_ERROR();
        return;
    }
}

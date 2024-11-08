#include "Error.h"
#include "Vector.h"
#include "Backup.h"
#include "ScratchBuf.h"

ErrorCode RunBackup(const char* argv[]);
ErrorCode RunRestore(const char* argv[]);

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    CHECK_ERROR(ScratchBufInit(MAX_PATH_SIZE));

    switch (argc)
    {
    case 3:
        err = RunBackup(argv);
        break;
    case 2:
        err = RunRestore(argv);
        break;
    default:
        err = ERROR_BAD_ARGS;
        LOG_ERROR();
        break;
    }

ERROR_CASE
    ScratchBufDtor();

    return err;
}

ErrorCode RunBackup(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    char* backupPath  = NULL;
    char* storagePath = NULL;

    backupPath = SanitizePath(argv[1]);
    if (!backupPath)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    storagePath = SanitizePath(argv[2]);
    if (!storagePath)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    CHECK_ERROR(Backup(backupPath, storagePath));

ERROR_CASE
    free(backupPath);
    free(storagePath);

    return err;
}

ErrorCode RunRestore(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    char* storagePath = NULL;

    storagePath = SanitizePath(argv[1]);
    if (!storagePath)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    Restore(storagePath);

ERROR_CASE
    free(storagePath);
    return err;
}

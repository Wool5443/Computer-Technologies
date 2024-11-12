#include "Error.h"
#include "Vector.h"
#include "Backup.h"
#include "ScratchBuf.h"

ErrorCode RunBackup(const char* argv[]);
ErrorCode RunRestore(const char* argv[]);

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    CHECK_ERROR(ScratchInit(MAX_PATH_SIZE));

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
    ScratchDtor();

    return err;
}

ErrorCode RunBackup(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    String backupPath  = {};
    String storagePath = {};

    backupPath = SanitizeDirectoryPath(argv[1]);
    if (!backupPath.data)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    storagePath = SanitizeDirectoryPath(argv[2]);
    if (!storagePath.data)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    CHECK_ERROR(Backup((StringSlice){ backupPath.size, backupPath.data },
                       (StringSlice){ storagePath.size, storagePath.data }));

ERROR_CASE
    free(backupPath.data);
    free(storagePath.data);

    return err;
}

ErrorCode RunRestore(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    String storagePath = {};

    storagePath = SanitizeDirectoryPath(argv[1]);
    if (!storagePath.data)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    Restore((StringSlice){ storagePath.size, storagePath.data });

ERROR_CASE
    free(storagePath.data);
    return err;
}

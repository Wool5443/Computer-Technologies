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

    ResultString backupPathRes = SanitizeDirectoryPath(argv[1]);
    CHECK_ERROR(backupPathRes.error);

    backupPath = backupPathRes.value;

    ResultString storagePathRes = SanitizeDirectoryPath(argv[1]);
    CHECK_ERROR(storagePathRes.error);
    storagePath = storagePathRes.value;

    CHECK_ERROR(Backup(StrCtorFromString(backupPath), StrCtorFromString(storagePath)));

ERROR_CASE
    StringDtor(&backupPath);
    StringDtor(&storagePath);

    return err;
}

ErrorCode RunRestore(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    String storagePath = {};

    ResultString storagePathRes = SanitizeDirectoryPath(argv[1]);
    CHECK_ERROR(storagePathRes.error);
    storagePath = storagePathRes.value;

    Restore(StrCtorFromString(storagePath));

ERROR_CASE
    free(storagePath.data);
    return err;
}

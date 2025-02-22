#include "Backup.h"
#include "ScratchBuf.h"
#include "IO.h"

ErrorCode RunBackup(const char* argv[]);
ErrorCode RunRestore(const char* argv[]);

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    LoggerInitConsole();

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
        fprintf(stdout, "Usage: to backup: %s <folder to backup> <folder for storage>\n", argv[0]);
        fprintf(stdout, "or to restore data: %s <folder for storage> \n", argv[0]);
        break;
    }

ERROR_CASE
    ScratchDtor();

    LoggerFinish();

    return err;
}

ErrorCode RunBackup(const char* argv[])
{
    ERROR_CHECKING();

    assert(argv);

    String backupPath  = {};
    String storagePath = {};

    ResultString backupPathRes = RealPath(argv[1]);
    CHECK_ERROR(backupPathRes.errorCode);

    backupPath = backupPathRes.value;

    ResultString storagePathRes = RealPath(argv[2]);
    CHECK_ERROR(storagePathRes.errorCode);
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

    ResultString storagePathRes = RealPath(argv[1]);
    CHECK_ERROR(storagePathRes.errorCode);
    storagePath = storagePathRes.value;

    Restore(StrCtorFromString(storagePath));

ERROR_CASE
    free(storagePath.data);
    return err;
}

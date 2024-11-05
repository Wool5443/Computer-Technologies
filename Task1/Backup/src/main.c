#include "Vector.h"
#include "Backup.h"
#include "ScratchBuf.h"

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    if (argc != 3)
    {
        LOG("BAD ARGS!\nInput backup and storage folder!\n");
        return ERROR_BAD_ARGS;
    }

    Backupper backupper = {};
    if ((err = ScratchBufInit(MAX_PATH_SIZE)))
    {
        goto cleanup;
    }

    ResultBackupper backupperRes = BackupperCtor(argv[1], argv[2]);
    if ((err = backupperRes.error))
    {
        LOG_IF_ERROR();
        goto cleanup;
    }

    backupper = backupperRes.value;

    if ((err = Backup(&backupper)))
    {
        LOG_IF_ERROR();
        goto cleanup;
    }
cleanup:
    BackupperDtor(&backupper);

    return err;
}

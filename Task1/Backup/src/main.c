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

    for (size_t i = 0, sz = VecSize(backupper.fileList); i < sz; i++)
    {
        FileEntry ent = backupper.fileList[i];
        printf("%s: %ld\n", ent.path, ent.updateDate);
    }
cleanup:
    BackupperDtor(&backupper);

    return err;
}

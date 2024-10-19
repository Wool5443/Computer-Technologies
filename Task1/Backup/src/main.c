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
    FileList  list = {};
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

    ResultFileList fileListRes = FileListCtor(&backupper);
    if ((err = fileListRes.error))
    {
        LOG_IF_ERROR();
        goto cleanup;
    }

    list = fileListRes.value;

    for (size_t i = 0; i < list.size; i++)
    {
        printf("%s\n", list.files[i]);
    }

cleanup:
    FileListDtor(&list);
    BackupperDtor(&backupper);

    return err;
}

#ifndef FILE_LIST_H
#define FILE_LIST_H

#include <time.h>
#include "Logger.h" // IWYU pragma: keep

#define MAX_FD 32

typedef struct
{
    char*  path;
    time_t updateDate;
} FileEntry;

typedef FileEntry* FileList;

DECLARE_RESULT(FileList);

ResultFileList  FileListCtor(const char dir[static 1]);
void            FileListDtor(FileList list);

time_t          FileEntryCompare(const void* a, const void* b);

FileEntry*      FindFileEntry(const FileList fileList, FileEntry entry);

#endif

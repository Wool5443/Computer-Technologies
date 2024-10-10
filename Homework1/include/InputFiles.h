#ifndef INPUT_FILES_H
#define INPUT_FILES_H

#include <time.h>
#include <stddef.h>
#include "Error.h"

typedef struct
{
    const char* command;
    time_t      delay;
} Command;

typedef struct
{
    Command* commands;
    size_t   size;
    /*----------------*/
    char*    m_buffer;
    /*----------------*/
} CommandList;

DECLARE_RESULT(CommandList);

void              CommandListDtor(CommandList* list);
ResultCommandList CommandListCtor(const char filePath[static 1]);

#endif // INPUT_FILES_H

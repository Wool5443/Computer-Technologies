#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

#define MAX_ARGS 32

typedef struct
{
    const char* command;
    int         delay;
    char*       args[MAX_ARGS + 1];
} Command;

typedef struct
{
    size_t   size;
    Command* commands;
    /*----------------*/
    char*    m_buffer;
    /*----------------*/
} CommandList;

#endif // COMMANDS_H

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

typedef struct
{
    const char* command;
    int         delay;
} Command;

typedef struct
{
    Command* commands;
    size_t   size;
    /*----------------*/
    char*    m_buffer;
    /*----------------*/
} CommandList;

#endif // COMMANDS_H

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
    size_t   size;
    Command* commands;
    /*----------------*/
    char*    m_buffer;
    /*----------------*/
} CommandList;

#endif // COMMANDS_H

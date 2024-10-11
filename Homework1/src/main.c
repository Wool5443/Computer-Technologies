#include <stdio.h>
#include <stdlib.h>
#include "InputFiles.h"
#include "Scheduler.h"

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "INVALID ARGUMENTS!!!\n");
        return -1;
    }

    ResultCommandList commandsRes = CommandListCtor(argv[1]);
    RETURN_ERROR_IF(commandsRes.error);

    CommandList commands = commandsRes.value;

    for (size_t i = 0; i < commands.size; i++)
    {
        printf("delay = %d command = <%s>\n", (int)commands.commands[i].delay, commands.commands[i].command);
    }

    CommandListDtor(&commands);

    return 0;
}

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

    ERROR_CHECKING();
    CommandList commands = {};
    Scheduler scheduler = {};

    ResultCommandList commandsRes = CommandListCtor(argv[1]);
    if (commandsRes.error)
    {
        err = commandsRes.error;
        goto cleanup;
    }

    commands = commandsRes.value;

    for (size_t i = 0; i < commands.size; i++)
    {
        printf("delay = %d command = <%s>\n", commands.commands[i].delay, commands.commands[i].command);
    }

    ResultScheduler schedulerRes = SchedulerCtor(commands.size);
    if (schedulerRes.error)
    {
        err = schedulerRes.error;
        goto cleanup;
    }

    scheduler = schedulerRes.value;

    for (size_t i = 0; i < commands.size; i++)
    {
        ScheduleCommand(&scheduler, &commands.commands[i]);
    }

    SchedulerJoin(&scheduler);

cleanup:
    CommandListDtor(&commands);
    SchedulerDtor(&scheduler);
    RETURN(err, err);
}

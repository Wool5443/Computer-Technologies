#include <stdio.h>
#include "InputFiles.h"
#include "Scheduler.h"

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    if (argc != 2)
    {
        fprintf(stderr, "INVALID ARGUMENTS!!!\n");
        return -1;
    }

    CommandList commands = {};
    Scheduler scheduler = {};

    ResultCommandList commandsRes = CommandListCtor(argv[1]);
    if ((err = commandsRes.errorCode))
        goto cleanup;

    commands = commandsRes.value;

    for (size_t i = 0; i < commands.size; i++)
        printf("delay = %d command = <%s>\n", commands.commands[i].delay, commands.commands[i].command);
    printf("\n");

    ResultScheduler schedulerRes = SchedulerCtor(commands.size);
    if ((err = schedulerRes.errorCode))
        goto cleanup;

    scheduler = schedulerRes.value;

    for (size_t i = 0; i < commands.size; i++)
    {
        err = ScheduleCommand(&scheduler, &commands.commands[i]);
        if (err)
            goto cleanup;
    }

    SchedulerJoin(&scheduler);

cleanup:
    CommandListDtor(&commands);
    SchedulerDtor(&scheduler);
    return err;
}

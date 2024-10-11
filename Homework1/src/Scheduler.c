#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include "Scheduler.h"
#include "Error.h"

void* execCommandThreadFunc(void* arg)
{
    assert(arg);

    Command cmd = *(Command*)arg;

    sleep(cmd.delay);

    printf("Running command %s after delay %d with args:\n", cmd.command, cmd.delay);
    for (size_t i = 0; i < MAX_ARGS; i++)
    {
        if (cmd.args[i])
            printf("%s\n", cmd.args[i]);
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(cmd.command, cmd.args);
    }
    printf("Has run command %s after delay %d\n", cmd.command, cmd.delay);

    return NULL;
}

ResultScheduler SchedulerCtor(size_t size)
{
    ERROR_CHECKING();

    pthread_t* cmdThreads = NULL;

    cmdThreads = (pthread_t*)calloc(size, sizeof(*cmdThreads));

    if (!cmdThreads)
    {
        err = ERROR_NO_MEMORY;
        goto cleanup;
    }

    return (ResultScheduler) {
        .error = EVERYTHING_FINE,
        .value = {
            .size = size,
            .threads = cmdThreads,
            .currentThread = 0,
        }
    };

cleanup:
    free(cmdThreads);

    ResultScheduler res = { err, {} };
    RETURN(res, err);
}

void SchedulerDtor(Scheduler scheduler[static 1])
{
    assert(scheduler);

    free(scheduler->threads);
}

ErrorCode ScheduleCommand(Scheduler scheduler[static 1], Command command[static 1])
{
    assert(scheduler);
    assert(command);

    if (scheduler->currentThread == scheduler->size)
    {
        RETURN_ERROR_IF(ERROR_INDEX_OUT_OF_BOUNDS);
    }

    pthread_t newThread = 0;

    if (pthread_create(&newThread, NULL, execCommandThreadFunc, command)
        != EVERYTHING_FINE)
    {
        RETURN_ERROR_IF(ERROR_BAD_THREAD);
    }

    scheduler->threads[scheduler->currentThread++] = newThread;

    return EVERYTHING_FINE;
}

ErrorCode SchedulerJoin(Scheduler scheduler[static 1])
{
    assert(scheduler);

    ERROR_CHECKING();

    pthread_t* threads = scheduler->threads;

    for (size_t i = 0, end = scheduler->currentThread; i < end; i++)
    {
        if (pthread_join(threads[i], NULL) != EVERYTHING_FINE)
        {
            fprintf(stderr, "%s\n", GetErrorName(ERROR_BAD_THREAD));
            err = ERROR_BAD_THREAD;
        }
    }

    return err;
}

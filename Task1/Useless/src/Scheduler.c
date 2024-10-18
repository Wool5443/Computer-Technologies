#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include "Scheduler.h"
#include "Error.h"

time_t StartTime = 0;

void* execCommandThreadFunc(void* arg)
{
    assert(arg);

    Command cmd = *(Command*)arg;

    sleep(cmd.delay);

    double waitTime = difftime(time(NULL), StartTime);

    printf("Waited for %g seconds\n"
           "Running command %s with delay %d with args:\n",
           waitTime, cmd.command, cmd.delay);

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
    printf("Has run command %s with delay %d\n\n", cmd.command, cmd.delay);

    return NULL;
}

ResultScheduler SchedulerCtor(size_t size)
{
    ERROR_CHECKING();

    StartTime = time(NULL);

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
    RETURN(res);
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

    ERROR_CHECKING();

    if (scheduler->currentThread == scheduler->size)
    {
        err = ERROR_INDEX_OUT_OF_BOUNDS;
        RETURN_ERROR_IF();
    }

    pthread_t newThread = 0;

    if (pthread_create(&newThread, NULL, execCommandThreadFunc, command)
        != EVERYTHING_FINE)
    {
        err = ERROR_BAD_THREAD;
        RETURN_ERROR_IF();
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
            err = ERROR_BAD_THREAD;
            LOG_IF_ERROR();
        }
    }

    return err;
}

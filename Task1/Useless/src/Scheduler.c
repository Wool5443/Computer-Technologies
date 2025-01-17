#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Scheduler.h"

DECLARE_RESULT_SOURCE(Scheduler);

time_t StartTime = 0;

void* execCommandThreadFunc(void* arg)
{
    assert(arg);

    Command cmd = *(Command*)arg;

    sleep(cmd.delay);

    pid_t pid = vfork();

    if (pid == 0)
    {
        execvp(cmd.command, cmd.args);
    }

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
        ERROR_LEAVE();
    }

    return ResultSchedulerCtor(
        (Scheduler)
        {
            .size = size,
            .threads = cmdThreads,
            .currentThread = 0,
        },
        err
    );

ERROR_CASE
    free(cmdThreads);

    return ResultSchedulerCtor((Scheduler){}, err);
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
        LogError();
        return err;
    }

    pthread_t newThread = 0;

    if (pthread_create(&newThread, NULL, execCommandThreadFunc, command)
        != EVERYTHING_FINE)
    {
        err = ERROR_BAD_THREAD;
        LogError();
        return err;
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
            LogError();
            return err;
        }
    }

    return err;
}

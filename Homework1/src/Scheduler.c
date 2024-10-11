#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "Scheduler.h"
#include "Error.h"

void* execCommandThreadFunc(void* arg)
{
    assert(arg);

    Command cmd = *(Command*)arg;

    sleep(cmd.delay);

    execv(cmd.command, (char* const*)cmd.command);

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
            .cmdThreads = cmdThreads,
            .currentThread = 0,
        }
    };

cleanup:
    free(cmdThreads);

    ResultScheduler res = { err, {} };
    RETURN(res, err);
}

ErrorCode ScheduleCommand(Scheduler* scheduler, Command* command)
{
    assert(scheduler);
    Scheduler sch = *scheduler;

    if (sch.currentThread == sch.size)
    {
        RETURN_ERROR_IF(ERROR_INDEX_OUT_OF_BOUNDS);
    }

    pthread_t newThread = 0;

    if (pthread_create(&newThread, NULL, execCommandThreadFunc, command)
        != EVERYTHING_FINE)
    {
        RETURN_ERROR_IF(ERROR_BAD_THREAD);
    }

    return EVERYTHING_FINE;
}

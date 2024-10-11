#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>
#include "Commands.h"
#include "Error.h"

typedef struct
{
    size_t     size;
    pthread_t* cmdThreads;
    size_t     currentThread;
} Scheduler;

DECLARE_RESULT(Scheduler);

ResultScheduler SchedulerCtor(size_t size);

ErrorCode ScheduleCommand(Scheduler* scheduler, Command* command);

#endif // SCHEDULER_H

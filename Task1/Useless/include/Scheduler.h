#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>

#include "Commands.h"
#include "Logger.h"

typedef struct
{
    size_t     size;
    pthread_t* threads;
    size_t     currentThread;
} Scheduler;

DECLARE_RESULT_HEADER(Scheduler);

ResultScheduler SchedulerCtor(size_t size);
void            SchedulerDtor(Scheduler scheduler[static 1]);

ErrorCode ScheduleCommand(Scheduler scheduler[static 1],
                          Command command[static 1]);

ErrorCode SchedulerJoin(Scheduler scheduler[static 1]);

#endif // SCHEDULER_H

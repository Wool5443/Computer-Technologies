#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>

#include "Vector.h"
#include "RunSim.h"

#define MAX_USER_INPUT 512

static const char** parseArgs(char string[static 1]);

static void* poller(void* arg);

atomic_size_t currentProgramsRunningAtomic = 0;
atomic_bool running = true;

ErrorCode RunSim(size_t maxPrograms)
{
    ERROR_CHECKING();

    char userInput[MAX_USER_INPUT + 1] = "";

    pthread_t pollingThread = {};
    pthread_create(&pollingThread, NULL, poller, NULL);

    while (fgets(userInput, MAX_USER_INPUT, stdin))
    {
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n')
        {
            userInput[len - 1] = '\0';
        }

        size_t currentProgramsRunning = atomic_load_explicit(&currentProgramsRunningAtomic, memory_order_acquire);
        if (currentProgramsRunning  >= maxPrograms)
        {
            fprintf(stdout, "Max running programs reached. Command \"%s\" ignored.\n", userInput);
            continue;
        }

        const char** args = parseArgs(userInput);

        if (!args)
        {
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            int ern = errno;
            err = ERROR_LINUX;
            LogError("fork error: %s", strerror(ern));
            VecDtor(args);
            ERROR_LEAVE();
        }
        else if (pid == 0)
        {
            execvp(userInput, (char**)args);

            int ern = errno;
            err = ERROR_LINUX;
            LogError("execvp error: %s", strerror(ern));
            VecDtor(args);
            ERROR_LEAVE();
        }

        atomic_fetch_add_explicit(&currentProgramsRunningAtomic, 1, memory_order_release);

        VecDtor(args);
    }

ERROR_CASE
    atomic_store_explicit(&running, false, memory_order_release);

    while (atomic_load_explicit(&currentProgramsRunningAtomic, memory_order_acquire))
    {
        wait(NULL);
        atomic_fetch_sub_explicit(&currentProgramsRunningAtomic, 1, memory_order_release);
    }

    return err;
}

static const char** parseArgs(char string[static 1])
{
    ERROR_CHECKING();

    assert(string);

    const char** args = NULL;

    char* current = strtok(string, " ");

    while (current)
    {
        CHECK_ERROR(VecAdd(args, current), "Error pushing %s to vector", current);
        current = strtok(NULL, " ");
    }

    CHECK_ERROR(VecAdd(args, NULL), "Error pushing NULL to vector");

    return args;

ERROR_CASE
    VecDtor(args);

    return NULL;
}

static void* poller(UNUSED void* arg)
{
    ERROR_CHECKING();

    while (atomic_load_explicit(&running, memory_order_acquire))
    {
        int status = 0;
        pid_t finishedPid = waitpid(-1, &status, WNOHANG);

        if (finishedPid > 0)
        {
            atomic_fetch_sub_explicit(&currentProgramsRunningAtomic, 1, memory_order_release);
        }
    }

    return NULL;
}

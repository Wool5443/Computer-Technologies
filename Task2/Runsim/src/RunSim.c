#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include "ScratchBuf.h"
#include "ProgramList.h"
#include "RunSim.h"

sem_t* prListSemaphoreGlobalPtr = NULL;
ProgramList* prListGlobalPtr = NULL;

#define MAX_USER_INPUT_SIZE 512

static void sigchildHandler(int signum, siginfo_t* siginfo, void* unused);

static ErrorCode runProgram(const char* args[]);

ErrorCode RunSim(size_t maxPrograms)
{
    ERROR_CHECKING();

    ScratchInit(1024);

    ProgramList prList = NULL;
    sem_t prListSemaphore = {};

    prListGlobalPtr = &prList;
    prListSemaphoreGlobalPtr = &prListSemaphore;

    if (sem_init(&prListSemaphore, 0, 1) == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("sem_init: %s", strerror(ern));
        ERROR_LEAVE();
    }

    ResultProgramList prListRes = ProgramListCtor(maxPrograms);

    if (prListRes.errorCode)
    {
        err = prListRes.errorCode;
        ERROR_LEAVE();
    }

    prList = prListRes.value;

    struct sigaction sa = {};
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = sigchildHandler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("sigaction failed: %s", strerror(ern));
        ERROR_LEAVE();
    }

    char userInput[MAX_USER_INPUT_SIZE + 1] = "";

    while (fgets(userInput, MAX_USER_INPUT_SIZE, stdin) || 0)
    {
        LogDebug("Got a string!");

        sem_wait(&prListSemaphore);

        size_t currentPrograms = VecSize(prList);

        LogDebug("Currently running %zu", currentPrograms);

        sem_post(&prListSemaphore);

        if (currentPrograms == maxPrograms)
        {
            fprintf(stdout, "Too many programs running, try again later\n");
            continue;
        }

        char* endLine = strchr(userInput, '\n');
        if (endLine) *endLine = '\0';

        const char* args[] = { userInput, NULL };

        runProgram(args);

        LogDebug("Successfully started program!");

        memset(userInput, '\0', MAX_USER_INPUT_SIZE + 1);
    }

ERROR_CASE
    ScratchDtor();
    ProgramListDtor(prList);
    sem_destroy(&prListSemaphore);

    return err;
}

static void sigchildHandler([[maybe_unused]] int signum,
                            siginfo_t* siginfo,
                            [[maybe_unused]] void* unused)
{
    ERROR_CHECKING();

    int status = 0;
    int pid = siginfo->si_pid;

    if (siginfo->si_code != CLD_EXITED)
    {
        err = ERROR_LINUX;
        LogError("wring si_code");
        ERROR_LEAVE();
    }
    else if (waitpid(pid, &status, 0) == -1)
    {
        err = ERROR_LINUX;
        LogError("waitpid() failed");
        ERROR_LEAVE();
    }
    else if (!WIFEXITED(status))
    {
        err = ERROR_LINUX;
        LogError("WIFEXITED was false");
        ERROR_LEAVE();
    }

    sem_wait(prListSemaphoreGlobalPtr);

    for (size_t i = 0, end = VecSize(*prListGlobalPtr); i < end; i++)
    {
        if ((*prListGlobalPtr)[i].id != pid) continue;

        (*prListGlobalPtr)[i] = (*prListGlobalPtr)[end - 1];

        LogDebug("Before pop %zu elements", VecSize(*prListGlobalPtr));

        VecPop(*prListGlobalPtr);

        LogDebug("After pop %zu elements", VecSize(*prListGlobalPtr));

        break;
    }

    sem_post(prListSemaphoreGlobalPtr);

ERROR_CASE
}

static ErrorCode runProgram(const char* args[])
{
    ERROR_CHECKING();

    assert(args);
    assert(prListGlobalPtr);
    assert(prListSemaphoreGlobalPtr);

    pid_t program = fork();

    if (program == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("%s", strerror(ern));
        ERROR_LEAVE();
    }
    else if (program == 0)
    {
        // CHILD
        ScratchClean();
        ScratchAppend("Running: ");

        const char** argsPtr = args;
        const char* arg = *argsPtr++;

        while (arg)
        {
            ScratchAppend(arg);
            ScratchAppendChar(' ');
            arg = *argsPtr++;
        }

        LogInfo("%s", ScratchGetStr().data);

        execvp(*args, (char**)args);
    }
    else
    {
        // PARENT
        sem_wait(prListSemaphoreGlobalPtr);

        VecAdd(*prListGlobalPtr, (Program){ program });

        sem_post(prListSemaphoreGlobalPtr);
    }

ERROR_CASE
    return err;
}

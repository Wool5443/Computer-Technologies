#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "Logger.h"
#include "ScratchBuf.h"
#include "ProgramList.h"
#include "RunSim.h"

ProgramList* prListGlobalPtr = NULL;

#define MAX_USER_INPUT_SIZE 512

static void sigchildHandler(int signum, siginfo_t* siginfo, void* unused);

static ErrorCode runProgram(const char* args[]);
static ErrorCode childFunction(const char* args[], int* pipeFd);
static ErrorCode parentFunction(const char* args[], int* pipeFd);
static const char** parseArgs(char string[static 1]);

ErrorCode RunSim(size_t maxPrograms)
{
    ERROR_CHECKING();

    ProgramList prList = NULL;
    prListGlobalPtr = &prList;

    if ((err = ScratchInit(1024)))
    {
        LogError("Unable to initialize ScratchBuffer");
        ERROR_LEAVE();
    }

    LogInfo("ScratchBuffer initialized");

    ResultProgramList prListRes = ProgramListCtor(maxPrograms);
    if (prListRes.errorCode)
    {
        err = prListRes.errorCode;
        ERROR_LEAVE();
    }
    prList = prListRes.value;
    LogInfo("ProgramList created with");

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
    LogInfo("SIGCHLD handler has been setup");

    if (prctl(PR_SET_CHILD_SUBREAPER, 1) == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("prctl error: %s", strerror(ern));
        ERROR_LEAVE();
    }

    char userInput[MAX_USER_INPUT_SIZE + 1] = "";

    while (fgets(userInput, MAX_USER_INPUT_SIZE, stdin) || 0)
    {
        size_t currentPrograms = VecSize(prList);

        LogDebug("Currently running %zu", currentPrograms);

        if (currentPrograms == maxPrograms)
        {
            fprintf(stdout, "Too many programs running, try again later\n");
            goto nextLoop;
        }

        char* endLine = strchr(userInput, '\n');
        if (endLine) *endLine = '\0';

        if (!*userInput) continue;

        const char** args = parseArgs(userInput);

        runProgram(args);

        VecDtor(args);

        for (size_t i = 0, end = VecSize(prList); i < end; i++)
        {
            fprintf(stderr, "%d ", prList[i].id);
        }
        fprintf(stderr, "\n");

nextLoop:
        memset(userInput, '\0', MAX_USER_INPUT_SIZE + 1);
    }

ERROR_CASE
    ScratchDtor();
    ProgramListDtor(prList);

    return err;
}

static void sigchildHandler([[maybe_unused]] int signum,
                            siginfo_t* siginfo,
                            [[maybe_unused]] void* unused)
{
    ERROR_CHECKING();

    LogInfo("Handling SIGCHLD");

    int status = 0;
    int pid = siginfo->si_pid;

    LogDebug("Popping %d", pid);

    if (siginfo->si_code != CLD_EXITED)
    {
        err = ERROR_LINUX;
        LogError("wrong si_code");
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

    for (size_t i = 0, end = VecSize(*prListGlobalPtr); i < end; i++)
    {
        LogDebug("pid: %d, current: %d", pid, (*prListGlobalPtr)[i].id);
        if ((*prListGlobalPtr)[i].id != pid) continue;

        (*prListGlobalPtr)[i] = (*prListGlobalPtr)[end - 1];

        VecPop(*prListGlobalPtr);

        LogDebug("After pop %zu elements", VecSize(*prListGlobalPtr));

        break;
    }

ERROR_CASE
}

static ErrorCode runProgram(const char* args[])
{
    ERROR_CHECKING();

    assert(args);
    assert(prListGlobalPtr);

    int pipeFd[2] = {};
    if (pipe(pipeFd) == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("Unable to initialize pipe: %s", strerror(ern));
        ERROR_LEAVE();
    }
    LogInfo("Pipe created: [%d, %d]", pipeFd[0], pipeFd[1]);

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
        err = childFunction(args, pipeFd);
    }
    else
    {
        err = parentFunction(args, pipeFd);
    }

ERROR_CASE
    if (pipeFd[0]) close(pipeFd[0]);
    if (pipeFd[1]) close(pipeFd[1]);
    return err;
}

static ErrorCode childFunction(const char* args[], int* pipeFd)
{
    ERROR_CHECKING();

    ScratchClean();
    if ((err = ScratchAppend("Running:")))
    {
        LogError("Error formatting");
        ERROR_LEAVE();
    }

    const char** argsPtr = args;
    const char* arg = *argsPtr++;

    while (arg)
    {
        if ((err = ScratchAppendChar(' ')))
        {
            LogError("Error formatting");
            ERROR_LEAVE();
        }
        if ((err = ScratchAppend(arg)))
        {
            LogError("Error formatting");
            ERROR_LEAVE();
        }
        arg = *argsPtr++;
    }

    LogInfo("%s", ScratchGetStr().data);

    pid_t program = fork();

    if (program == 0)
    {
        // CHILD
        if (execvp(*args, (char**)args) == -1)
        {
            if (close(pipeFd[0]) == -1) // close read
            {
                int ern = errno;
                err = ERROR_LINUX;
                LogError("Close error: %s", strerror(ern));
                ERROR_LEAVE();
            }

            pid_t badProgram = -1;
            ssize_t writeBytes = write(pipeFd[1], &badProgram, sizeof(badProgram));

            if (writeBytes == -1)
            {
                int ern = errno;
                err = ERROR_LINUX;
                LogError("Write error: %s", strerror(ern));
                ERROR_LEAVE();
            }
        }
    }

    LogInfo("Child be like %d", program);

    ssize_t writeBytes = write(pipeFd[1], &program, sizeof(program));

    if (writeBytes == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("Write error: %s", strerror(ern));
        ERROR_LEAVE();
    }

ERROR_CASE
    return err;
}

static ErrorCode parentFunction(const char* args[], int* pipeFd)
{
    ERROR_CHECKING();

    if (close(pipeFd[1]) == -1) // close write
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("Close error: %s", strerror(ern));
        ERROR_LEAVE();
    }

    pid_t program = -1;
    ssize_t readBytes = read(pipeFd[0], &program, sizeof(program));

    if (readBytes == -1)
    {
        int ern = errno;
        err = ERROR_LINUX;
        LogError("Read error: %s", strerror(ern));
        ERROR_LEAVE();
    }
    else if (program == -1)
    {
        fprintf(stdout, "Unable to run program %s", *args);
        ERROR_LEAVE();
    }

    LogInfo("Adding %d", program);

    err = VecAdd(*prListGlobalPtr, (Program){ program });
    if (err)
    {
        LogError("Could not push new program to vector");
        ERROR_LEAVE();
    }

    LogDebug("After VecAdd: %zu", VecSize(*prListGlobalPtr));

ERROR_CASE
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
        if ((err = VecAdd(args, current)))
        {
            LogError("Error parsing args");
            ERROR_LEAVE();
        }
        current = strtok(NULL, " ");
    }

    if ((err = VecAdd(args, NULL)))
    {
        LogError("Error parsing args");
        ERROR_LEAVE();
    }

    return args;

ERROR_CASE
    VecDtor(args);

    return NULL;
}

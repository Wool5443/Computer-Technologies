#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdatomic.h>

#include "Vector.h"
#include "RunSim.h"

#define MAX_USER_INPUT 512

static const char** parseArgs(char string[static 1]);

size_t running = 0;

ErrorCode RunSim(size_t maxPrograms)
{
    ERROR_CHECKING();

    char userInput[MAX_USER_INPUT + 1] = "";
    size_t running = 0;

    while (fgets(userInput, MAX_USER_INPUT, stdin))
    {
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n')
        {
            userInput[len - 1] = '\0';
        }

        if (running >= maxPrograms)
        {
            fprintf(stdout, "Max running programs reached. Command \"%s\" ignored.\n", userInput);
            goto nextLoop;
        }

        const char** args = parseArgs(userInput);

        if (!args)
        {
            goto nextLoop;
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

        LogInfo("before running: %zu", running);
        running++;
        LogInfo("after running: %zu", running);

        VecDtor(args);

nextLoop:
        while (running > 0)
        {
            int status = 0;
            pid_t finishedPid = waitpid(-1, &status, WNOHANG);

            if (finishedPid == -1)
            {
                int ern = errno;
                err = ERROR_LINUX;
                LogError("waitpid error: %s", strerror(ern));
                ERROR_LEAVE();
            }
            else if (finishedPid > 0)
            {
                running--;
            }
            else if (finishedPid == 0)
            {
                break;
            }
        }
    }

    while (running > 0)
    {
        wait(NULL);
        running--;
    }

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

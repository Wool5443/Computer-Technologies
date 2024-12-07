#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include "RunSim.h"

#define MAX_USER_INPUT 512

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

        pid_t pid = fork();
        if (pid < 0)
        {
            int ern = errno;
            err = ERROR_LINUX;
            LogError("fork error: %s", strerror(ern));
            ERROR_LEAVE();
        }
        else if (pid == 0)
        {
            char* args[] = { userInput, NULL };
            execvp(userInput, args);

            int ern = errno;
            err = ERROR_LINUX;
            LogError("execvp error: %s", strerror(ern));
            ERROR_LEAVE();
        }
        else
        {
            running++;
        }

nextLoop:
        while (running > 0)
        {
            int status = 0;
            pid_t finishedPid = waitpid(-1, &status, WNOHANG);

            if (finishedPid > 0)
            {
                running--;
            }
            else if (finishedPid == 0)
            {
                break;
            }
            else
            {
                int ern = errno;
                err = ERROR_LINUX;
                LogError("waitpid error: %s", strerror(ern));
                ERROR_LEAVE();
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

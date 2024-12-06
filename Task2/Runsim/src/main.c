#include <stdlib.h>
#include <stdatomic.h>
#include "RunSim.h"
#include "Logger.h" // IWYU pragma: keep

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    // LoggerInit("log.txt");
    LoggerInitConsole();

    if (argc != 2)
    {
        err = ERROR_BAD_ARGS;
        LogError("Give max number of processes in args!!!");
        ERROR_LEAVE();
    }

    size_t maxPrograms = atoi(argv[1]);

    if ((err = RunSim(maxPrograms)))
    {
        ERROR_LEAVE();
    }

ERROR_CASE
    LoggerFinish();
    return err;
}

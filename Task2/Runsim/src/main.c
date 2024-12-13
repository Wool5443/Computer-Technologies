#include <stdio.h>
#include <stdlib.h>
#include "Logger.h"
#include "RunSim.h"

#define MAX_CMD_LEN 256

int main(int argc, char* argv[])
{
    ERROR_CHECKING();

    LoggerInit("log.txt");

    if (argc != 2)
    {
        fprintf(stdout, "Usage: %s <max running programs>\n", argv[0]);
        err = ERROR_BAD_ARGS;
        ERROR_LEAVE();
    }

    int maxPrograms = atoi(argv[1]);
    if (maxPrograms <= 0)
    {
        fprintf(stdout, "Error: max running programs should be a positive integer\n");
        err = ERROR_BAD_ARGS;
        ERROR_LEAVE();
    }

    err = RunSim(maxPrograms);

ERROR_CASE
    LoggerFinish();

    return err;
}

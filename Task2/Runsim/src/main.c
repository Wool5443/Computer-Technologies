#include <stdio.h>
#include <stdlib.h>
#include "RunSim.h"

#define MAX_CMD_LEN 256

int main(int argc, char* argv[])
{
    ERROR_CHECKING();

    if (argc != 2)
    {
        fprintf(stdout, "Usage: %s <max_running_programs>\n", argv[0]);
        return ERROR_BAD_ARGS;
    }

    int maxPrograms = atoi(argv[1]);
    if (maxPrograms <= 0)
    {
        fprintf(stdout, "Error: max_running_programs should be a positive integer\n");
        return ERROR_BAD_ARGS;
    }

    return RunSim(maxPrograms);
}

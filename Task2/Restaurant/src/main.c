#include <stdio.h>

#include "Logger.h"
#include "Restaurant.h"

int main(int argc, const char* argv[])
{
    ERROR_CHECKING();

    LoggerInit("log.txt");

    if (argc != 3)
    {
        fprintf(stderr, "WRONG ARGS!!! Correct usage: %s <orders-file> <times-file>", argv[0]);
        ERROR_LEAVE();
    }

    err = RunRestaurant(argv[1], argv[2]);

ERROR_CASE
    LoggerFinish();

    return err;
}

#include <stdlib.h>
#include <sys/wait.h>
#include "ProgramList.h"

ResultProgramList ProgramListCtor(size_t maxPrograms)
{
    ERROR_CHECKING();

    ProgramList list = NULL;

    if (maxPrograms == 0)
    {
        err = ERROR_BAD_VALUE;
        LogError("maxPrograms = %zu is invalid!", maxPrograms);
        ERROR_LEAVE();
    }

    VecExpand(list, maxPrograms);

    if (!list)
    {
        err = ERROR_NO_MEMORY;
        LogError("Could not initialize program list!");
        ERROR_LEAVE();
    }

    return (ResultProgramList)
    {
        list,
        err,
    };

ERROR_CASE
    VecDtor(list);
    return (ResultProgramList)
    {
        {},
        err,
    };
}

void ProgramListDtor(ProgramList programList)
{
    for (size_t i = 0, end = VecSize(programList); i < end; i++)
    {
        // waitpid(programList[i].id, NULL, WUNTRACED);
    }
    VecDtor(programList);
}

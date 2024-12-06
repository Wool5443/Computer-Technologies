#ifndef PROGRAM_LIST_H
#define PROGRAM_LIST_H

#include <unistd.h>
#include "Logger.h" // IWYU pragma: keep
#include "Vector.h" // IWYU pragma: keep

typedef struct
{
    pid_t id;
} Program;

typedef Program* ProgramList;

DECLARE_RESULT(ProgramList);

ResultProgramList ProgramListCtor(size_t maxPrograms);

void ProgramListDtor(ProgramList programList);

#endif // PROGRAM_LIST_H

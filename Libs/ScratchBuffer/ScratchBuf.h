#ifndef SCRATCH_BUF_H
#define SCRATCH_BUF_H

#include "StringSlice.h"
#include "Error.h"

ErrorCode               ScratchBufInit(size_t capacity);
void                    ScratchBufDtor();
void                    ScratchBufClean();

size_t                  ScratchGetSize();
char*                   ScratchGetStr();

ErrorCode               ScratchAppendChar(char c);
ErrorCode               ScratchAppendSlice(StringSlice slice);
static inline ErrorCode ScratchAppendStr(const char* str)
{
    if (!str) return EVERYTHING_FINE;
    return ScratchAppendSlice(StringSliceCtor(str));
}

void ScratchBufPop();

#endif

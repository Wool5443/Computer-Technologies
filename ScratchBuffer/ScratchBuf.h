#ifndef SCRATCH_BUF_H
#define SCRATCH_BUF_H

#include "StringSlice.h"
#include "Error.h"

typedef struct SScratchBuf_ ScratchBuf;

ErrorCode ScratchBufInit(size_t capacity);
void      ScratchBufDtor();
void      ScratchBufClean();

size_t ScratchBufSize();
ErrorCode ScratchBufAppendChar(char c);
ErrorCode ScratchBufAppendSlice(StringSlice slice);
static inline ErrorCode ScratchBufAppendStr(const char* str)
{
    if (!str) return EVERYTHING_FINE;

    return ScratchBufAppendSlice((StringSlice){ strlen(str), str });
}

#endif

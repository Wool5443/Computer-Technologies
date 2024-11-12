#ifndef SCRATCH_BUF_H
#define SCRATCH_BUF_H

#include "StringSlice.h"
#include "Error.h"

#define INLINE static inline
#if defined(__GNUC__) || defined(__clang__)
    #define MAYBE_UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

ErrorCode ScratchInit(size_t capacity);
void      ScratchDtor();
void      ScratchClean();

size_t    ScratchGetSize();
char*     ScratchGetStr();

ErrorCode ScratchAppendChar(char c);
ErrorCode ScratchAppendSlice(StringSlice slice);

INLINE MAYBE_UNUSED ErrorCode ScratchAppendStr(const char* str)
{
    if (!str) return EVERYTHING_FINE;
    return ScratchAppendSlice(StringSliceCtor(str));
}

void ScratchPop();

#endif

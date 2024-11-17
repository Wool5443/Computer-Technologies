#ifndef SCRATCH_BUF_H
#define SCRATCH_BUF_H

#include "String.h"

#define INLINE static inline
#if defined(__GNUC__) || defined(__clang__)
    #define MAYBE_UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

ErrorCode ScratchInit(size_t capacity);
void      ScratchDtor();
void      ScratchClean();

size_t       ScratchGetSize();
char*        ScratchGet();
Str          ScratchGetStr();
ResultString ScratchGetString();

ErrorCode ScratchAppendStr(Str slice);

INLINE MAYBE_UNUSED ErrorCode ScratchAppendChar(char c)
{
    char chstr[] = { c, '\0'};
    Str chslice = StrCtor(chstr);

    return ScratchAppendStr(chslice);
}

INLINE MAYBE_UNUSED ErrorCode ScratchAppendString(const String string)
{
    return ScratchAppendStr(StrCtorFromString(string));
}

INLINE MAYBE_UNUSED ErrorCode ScratchAppend(const char* string)
{
    return ScratchAppendStr(StrCtor(string));
}

void ScratchPop(size_t count);

#endif

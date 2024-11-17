#include "ScratchBuf.h"
#include <math.h>

#define CHECK_SCRATCH_STATE()                                       \
do                                                                  \
{                                                                   \
    if (!scratchString.data)                                        \
    {                                                               \
        LOG("PLEASE, INITIALIZE SCRATCH BUFFER FIRST!!!!\n");       \
        abort();                                                    \
    }                                                               \
} while (0)

static String scratchString;

ErrorCode ScratchInit(size_t capacity)
{
    ERROR_CHECKING();

    assert(capacity != 0 && "Capacity can't be zero!");

    ResultString strRes = StringCtorCapacity(capacity);

    err = strRes.error;
    RETURN_ERROR_IF();

    scratchString = strRes.value;

    return err;
}

void ScratchDtor()
{
    StringDtor(&scratchString);
}

void ScratchClean()
{
    CHECK_SCRATCH_STATE();

    memset(scratchString.data, '\0', scratchString.capacity);

    scratchString.size = 0;
}

ErrorCode ScratchAppendStr(const Str string)
{
    CHECK_SCRATCH_STATE();

    ERROR_CHECKING();

    if (!string.data) return err;

    err = StringAppendStr(&scratchString, string);

    RETURN(err);
}

#include <stdint.h>
#include <stdlib.h>
#include "ScratchBuf.h"

typedef struct
{
    size_t size;
    size_t capacity;
    char*  data;
} ScratchBuf;

static ScratchBuf scratchBuf = {};

ErrorCode ScratchInit(size_t capacity)
{
    ERROR_CHECKING();

    if (capacity == 0 || capacity > UINT32_MAX)
    {
        err = ERROR_BAD_VALUE;
        RETURN(err);
    }

    char* data = calloc(capacity, 1);
    if (!data)
    {
        err = ERROR_NO_MEMORY;
        RETURN(err);
    }

    scratchBuf = (ScratchBuf) {
        .size = 0,
        .capacity = capacity,
        .data = data,
    };

    return err;
}

void ScratchDtor()
{
    free(scratchBuf.data);
}

void ScratchClean()
{
    memset(scratchBuf.data, 0, scratchBuf.size);
    scratchBuf.size = 0;
}

size_t ScratchGetSize()
{
    return scratchBuf.size;
}

char* ScratchGetStr()
{
    return scratchBuf.data;
}

ErrorCode ScratchAppendChar(char c)
{
    ERROR_CHECKING();

    if (scratchBuf.size + 1 >= scratchBuf.capacity)
    {
        err = ERROR_INDEX_OUT_OF_BOUNDS;
        RETURN(err);
    }

    scratchBuf.data[scratchBuf.size++] = c;
    scratchBuf.data[scratchBuf.size] = '\0';

    return err;
}

ErrorCode ScratchAppendSlice(StringSlice slice)
{
    ERROR_CHECKING();

    if (!slice.data)
    {
        err = ERROR_NULLPTR;
        RETURN(err);
    }

    if (scratchBuf.size + slice.size + 1 > scratchBuf.capacity)
    {
        err = ERROR_INDEX_OUT_OF_BOUNDS;
        RETURN(err);
    }

    memcpy(scratchBuf.data + scratchBuf.size, slice.data, slice.size);
    scratchBuf.size += slice.size;

    scratchBuf.data[scratchBuf.size] = '\0';

    return err;
}

void ScratchPop()
{
    if (scratchBuf.size == 0) return;

    scratchBuf.size--;
    scratchBuf.data[scratchBuf.size] = '\0';
}

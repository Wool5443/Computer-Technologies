#include <stdint.h>
#include <stdlib.h>
#include "ScratchBuf.h"

struct SScratchBuf_
{
    size_t size;
    size_t capacity;
    char*  data;
};

static ScratchBuf scratchBuf = {};

ErrorCode ScratchBufInit(size_t capacity)
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

void ScratchBufDtor()
{
    free(scratchBuf.data);
}

void ScratchBufClean()
{
    scratchBuf.size = 0;
    scratchBuf.data[0] = '\0';
}

size_t ScratchBufSize()
{
    return scratchBuf.size;
}

ErrorCode ScratchBufAppendChar(char c)
{
    ERROR_CHECKING();
    if (scratchBuf.size == scratchBuf.capacity)
    {
        err = ERROR_INDEX_OUT_OF_BOUNDS;
        RETURN(err);
    }

    scratchBuf.data[scratchBuf.size++] = c;

    return err;
}

ErrorCode ScratchBufAppendSlice(StringSlice slice)
{
    ERROR_CHECKING();

    if (!slice.data) return err;

    if (scratchBuf.size + slice.size > scratchBuf.capacity)
    {
        err = ERROR_INDEX_OUT_OF_BOUNDS;
        RETURN(err);
    }

    memcpy(scratchBuf.data + scratchBuf.size, slice.data, slice.size);
    scratchBuf.size += slice.size;

    return err;
}

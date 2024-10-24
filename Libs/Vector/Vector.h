#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Error.h"

#define DEFAULT_CAPACITY 8

typedef struct
{
    size_t capacity;
    size_t size;
    char   data[];
} VHeader_;

#define INLINE static inline
#if defined(__GNUC__) || defined(__clang__)
    #define MAYBE_UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

#define GET_HEADER(ptr) (&((VHeader_*)(ptr))[-1])

INLINE MAYBE_UNUSED void* VecCtor(size_t elemSize, size_t capacity)
{
    ERROR_CHECKING();

    VHeader_* header = calloc(capacity * elemSize + sizeof(VHeader_), 1);
    if (!header)
    {
        err = ERROR_NO_MEMORY;
        LOG_IF_ERROR();
        return NULL;
    }

    header->capacity = capacity;

    return &header[1];
}

INLINE MAYBE_UNUSED void VecDtor(void* vec)
{
    assert(vec);
    if (vec) free(GET_HEADER(vec));
}

INLINE MAYBE_UNUSED size_t VecSize(void* vec)
{
    assert(vec);

    VHeader_* header = GET_HEADER(vec);
    return header->size;
}

INLINE MAYBE_UNUSED void* VecRealloc(void* vec, size_t elemSize)
{
    if (!vec)
        return VecCtor(elemSize, DEFAULT_CAPACITY);

    VHeader_* header = GET_HEADER(vec);

    if (header->size < header->capacity)
        return vec;

    size_t newCap = header->capacity * 3 / 2;

    void* newVec = VecCtor(elemSize, newCap);
    if (!newVec) return vec;

    memcpy(newVec, vec, elemSize * header->size);

    VHeader_* newHeader = GET_HEADER(newVec);
    *newHeader = *header;
    free(header);

    return newVec;
}

#define VecAdd(vec, value)                                                      \
do                                                                              \
{                                                                               \
    void* temp = VecRealloc((vec), sizeof(*vec));                               \
    if (!temp) break;                                                           \
    (vec) = temp;                                                               \
    VHeader_* header = GET_HEADER(vec);                                         \
    vec[header->size++] = value;                                                \
} while (0)

INLINE MAYBE_UNUSED void VecPop(void* vec)
{
    assert(vec);

    VHeader_* header = GET_HEADER(vec);
    header->size--;
}

#endif

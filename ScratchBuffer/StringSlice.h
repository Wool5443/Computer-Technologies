#ifndef STRING_SLICE_H_
#define STRING_SLICE_H_

#include <stddef.h>
#include <string.h>

typedef struct
{
    size_t size;
    const char* data;
} StringSlice;

[[maybe_unused]] static inline StringSlice StringSliceCtor(const char* data)
{
    if (!data)
        return (StringSlice){};

    return (StringSlice) {
        .size = strlen(data),
        .data = data,
    };
}

#endif

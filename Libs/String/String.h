#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "Error.h"

#define INLINE static inline
#if defined(__GNUC__) || defined(__clang__)
    #define MAYBE_UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

typedef struct
{
    char*  data;
    size_t size;
    size_t capacity;
} String;

typedef struct
{
    const char* data;
    size_t size;
} str;

DECLARE_RESULT(String);

INLINE MAYBE_UNUSED ResultString StringCtor(const char* string)
{
    ERROR_CHECKING();

    char* data = NULL;

    if (!string) return (ResultString){};

    size_t size = strlen(string);

    if (size == 0) return (ResultString){};

    data = calloc(size + 1, 1);

    if (!data)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    memcpy(data, string, size);

    return (ResultString)
    {
        err,
        (String)
        {
            .data = data,
            .size = size,
            .capacity = size,
        },
    };

ERROR_CASE
    free(data);

    return (ResultString){ err, (String){} };
}

INLINE MAYBE_UNUSED str StrCtor(const char* string)
{
    if (!string) return (str){};

    size_t size = strlen(string);

    if (size == 0) return (str){};

    return (str)
    {
        .data = string,
        .size = size,
    };
}

#endif // STRING_H_

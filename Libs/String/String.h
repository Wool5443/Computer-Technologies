#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

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
} Str;

DECLARE_RESULT(String);
DECLARE_RESULT(Str);

INLINE MAYBE_UNUSED Str StrCtor(const char* string)
{
    if (!string) return (Str){};

    size_t size = strlen(string);

    if (size == 0) return (Str){};

    return (Str)
    {
        .data = string,
        .size = size,
    };
}

INLINE MAYBE_UNUSED ResultString StringCtorFromStr(Str string)
{
    ERROR_CHECKING();

    char* data = NULL;

    if (!string.data) return (ResultString){};
    if (string.size == 0) return (ResultString){};

    data = calloc(string.size + 1, 1);

    if (!data)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    memcpy(data, string.data, string.size);

    return (ResultString)
    {
        err,
        (String)
        {
            .data = data,
            .size = string.size,
            .capacity = string.size,
        },
    };

ERROR_CASE
    free(data);

    return (ResultString){ err, (String){} };
}

INLINE MAYBE_UNUSED ResultString StringCtor(const char* string)
{
    return StringCtorFromStr(StrCtor(string));
}

INLINE MAYBE_UNUSED void StringDtor(String* string)
{
    if (!string) return;

    free(string->data);
}

INLINE MAYBE_UNUSED ErrorCode StringRealloc(String this[static 1], size_t newCapacity)
{
    ERROR_CHECKING();

    assert(this);
    assert(newCapacity);

    char* newData = NULL;

    if (this->capacity >= newCapacity) return EVERYTHING_FINE;

    newData = realloc(this->data, newCapacity);

    if (!newData)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    *this = (String)
    {
        .data = newData,
        .size = this->size,
        .capacity = newCapacity
    };

    return err;

ERROR_CASE
    free(newData);

    return err;
}

INLINE MAYBE_UNUSED ErrorCode StringAppendStr(String this[static 1], const Str string)
{
    ERROR_CHECKING();

    assert(this);

    if (!string.data) return EVERYTHING_FINE;
    if (string.size == 0) return EVERYTHING_FINE;

    size_t newSize = this->size + string.size;

    if (newSize > this->capacity)
    {
        err = StringRealloc(this, this->capacity * 3 / 2);
        if (err)
        {
            LOG_ERROR();
            ERROR_LEAVE();
        }
    }

    memcpy(this->data + this->size, string.data, string.size);

    return err;

ERROR_CASE
    return err;
}

INLINE MAYBE_UNUSED ErrorCode StringAppendString(String this[static 1], const String string)
{
    return StringAppendStr(this, (Str){ string.data, string.size });
}

INLINE MAYBE_UNUSED ErrorCode StringAppendChar(String this[static 1], char ch)
{
    char chstr[2] = { ch, '\0'};
    return StringAppendStr(this, (Str){ chstr, 1 });
}

INLINE MAYBE_UNUSED ResultStr StringSlice(const String this[static 1], size_t startIdx, size_t endIdx)
{
    ERROR_CHECKING();

    assert(this);

    if (startIdx >= this->size || endIdx >= this->size
        || endIdx < startIdx)
    {
        err = ERROR_BAD_ARGS;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    return (ResultStr)
    {
        err,
        (Str){ .data = this->data + startIdx, .size = endIdx - startIdx },
    };

ERROR_CASE

    return (ResultStr){ err, (Str){} };
}

#endif // STRING_H_

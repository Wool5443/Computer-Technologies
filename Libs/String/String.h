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
} str;

DECLARE_RESULT(String);

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

INLINE MAYBE_UNUSED ResultString StringCtorFromStr(str string)
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

INLINE MAYBE_UNUSED ErrorCode StringRealloc(String* this, size_t newCapacity)
{
    ERROR_CHECKING();

    assert(this);
    assert(newCapacity);

    char* newData = NULL;

    String string = *this;

    if (string.capacity >= newCapacity) return EVERYTHING_FINE;

    newData = realloc(string.data, newCapacity);

    if (!newData)
    {
        err = ERROR_NO_MEMORY;
        LOG_ERROR();
        ERROR_LEAVE();
    }

    string.data = newData;
    string.size = newCapacity;

    *this = string;

    return err;

ERROR_CASE
    free(newData);

    return err;
}

INLINE MAYBE_UNUSED ErrorCode StringAppendStr(String* this, const str string)
{
    ERROR_CHECKING();

    assert(this);

    if (!string.data) return EVERYTHING_FINE;
    if (string.size == 0) return EVERYTHING_FINE;

    String newString = *this;

    size_t newSize = newString.size + string.size;

    if (newSize > newString.capacity)
    {
        err = StringRealloc(&newString, newString.capacity * 3 / 2);
        if (err)
        {
            LOG_ERROR();
            ERROR_LEAVE();
        }
    }

    memcpy(newString.data + newString.size, string.data, string.size);

    *this = newString;

    return err;

ERROR_CASE
    return err;
}

INLINE MAYBE_UNUSED ErrorCode StringAppendString(String* this, const String string)
{
    return StringAppendStr(this, (str){ string.data, string.size });
}

INLINE MAYBE_UNUSED ErrorCode StringAppendChar(String* this, char ch)
{
    char chstr[2] = { ch, '\0'};
    return StringAppendStr(this, (str){ chstr, 1 });
}

#endif // STRING_H_

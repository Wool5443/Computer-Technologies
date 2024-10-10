#ifndef UTILS_H
#define UTILS_H

#include <stdio.h> // IWYU pragma: keep
#include <stddef.h>

typedef enum
{

#define DEF_ERROR(code) \
code,

#include "ErrorGen.hpp"

#undef DEF_ERROR

} ErrorCode;

const char* GetErrorName(ErrorCode err);

#define DECLARE_RESULT(Type)                        \
typedef struct                                      \
{                                                   \
    ErrorCode error;                                \
    Type value;                                     \
} Result ## Type

#define RETURN(retval, err)                         \
do                                                  \
{                                                   \
    fprintf(stderr, "%s in %s:%zu in %s\n",         \
            GetErrorName(err),                      \
            __FILE__,                               \
            (size_t)__LINE__,                       \
            __PRETTY_FUNCTION__                     \
           );                                       \
    return retval;                                  \
} while (0)

#define RETURN_ERROR_IF(err)                        \
do                                                  \
{                                                   \
    if (err)                                        \
        RETURN(err, err);                           \
} while (0)

#endif // UTILS_H

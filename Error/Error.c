#include "Error.h"

const char* GetErrorName(ErrorCode err)
{
    switch (err)
    {

#define DEF_ERROR(code) \
case code: return #code;

#include "ErrorGen.hpp"

#undef DEF_ERROR

        default:
            return "ERROR_BAD_ERROR_CODE";

    }
}

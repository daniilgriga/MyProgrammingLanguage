#include <stdio.h>

#include "errors.h"

const char* ErrorsMessenger (enum Errors status)
{
    switch (status)
    {
        case         NO_ERROR: return "No errors, for real";
        case   NULL_PTR_ERROR: return "NULL pointer detected";
        case  FILE_OPEN_ERROR: return "File failed to open";

        default:               return "UNDEFINED ERROR";
    }
}

#include <stdio.h>

#include "errors.h"

const char* ErrorsMessenger (enum Errors status)
{
    switch (status)
    {
        case NO_ERROR:          return "No errors, for real";
        case NULL_PTR_ERROR:    return "NULL pointer detected";
        case FILE_OPEN_ERROR:   return "File failed to open";
        case FILE_CLOSE_ERROR:  return "File failed to close";
        case FWRITE_ERROR:      return "fwrite not worked well";
        case CALLOC_ERROR:      return "error in calloc";

        default:               return "UNDEFINED ERROR";
    }
}

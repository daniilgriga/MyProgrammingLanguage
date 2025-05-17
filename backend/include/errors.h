#pragma once

#include "color.h"

enum Errors
{
    NO_ERROR,
    NULL_PTR_ERROR,
    FILE_OPEN_ERROR,
};

const char* ErrorsMessenger (enum Errors status);

#define ERROR_CHECK_RET_STATUS(status)                                                                      \
    {                                                                                                       \
        enum Errors temp = status;                                                                          \
        if (temp != NO_ERROR)                                                                               \
        {                                                                                                   \
            fprintf (stderr, "\n" RED_TEXT ("ERROR <%d>:") " %s, " PURPLE_TEXT ("%s: %s: line %d.") "\n",   \
                     (int)temp, ErrorsMessenger(temp), __FILE__, __FUNCTION__, __LINE__);                   \
            return temp;                                                                                    \
        }                                                                                                   \
    }

#define ERROR_MESSAGE(status)                                                                               \
    {                                                                                                       \
        enum Errors temp = status;                                                                          \
        if (temp != NO_ERROR)                                                                               \
        {                                                                                                   \
            fprintf (stderr, "\n" RED_TEXT ("ERROR <%d>:") " %s, " PURPLE_TEXT ("%s: %s: line %d.") "\n",   \
                     (int)temp, ErrorsMessenger(temp), __FILE__, __FUNCTION__, __LINE__);                   \
        }                                                                                                   \
    }

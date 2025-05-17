#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "file.h"
#include "errors.h"

FILE* OpenFile (const char* filename, const char* mode)
{
    assert (filename && "filename is NULL in OpenFile" "\n");
    assert (   mode  &&  "mode " "is NULL in OpenFile" "\n");

    FILE* file = fopen (filename, mode);
    if ( file == NULL )
    {
        fprintf (stderr, "\n" "Could not find the '%s' to be opened!" "\n", filename);
        return NULL;
    }

    return file;
}

enum Errors CloseFile (FILE* file_ptr)
{
    assert (file_ptr && "file_ptr is NULL in CloseFile" "\n");

    if ( fclose (file_ptr) == EOF )
    {
        perror ("Error occured");
        ERROR_CHECK_RET_STATUS (FILE_CLOSE_ERROR)
    }

    return NO_ERROR;
}

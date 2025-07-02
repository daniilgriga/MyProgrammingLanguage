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

#define FILE_SIZE_CHECK(status)   if ( status )                                         \
                                  {                                                     \
                                      perror ("The following error occurred");          \
                                      ERROR_MESSAGE (FILE_SIZE_ERROR)                   \
                                      return -1L;                                       \
                                  }

long FileSize (FILE* file_ptr)
{
    assert (file_ptr && "file_ptr is NULL in FileSize" "\n");

    long curr_pos = ftell (file_ptr);
    FILE_SIZE_CHECK (curr_pos == -1L)

    FILE_SIZE_CHECK ( fseek (file_ptr, 0L, SEEK_END) )

    long number_symbols = ftell (file_ptr);
    FILE_SIZE_CHECK ( number_symbols == -1L && number_symbols <= 0 )

    FILE_SIZE_CHECK ( fseek (file_ptr, curr_pos, SEEK_SET) )

    return number_symbols;
}

#undef FILE_SIZE_CHECK

char* ReadInBuffer (FILE* file_ptr, const long num_symb)
{
    char* buffer = (char*) calloc ( (size_t) num_symb + 1, sizeof (char) );        // EOF -> +1
    if (buffer == NULL)
        return NULL;

    size_t read_symb = fread (buffer, sizeof (buffer[0]), (size_t) num_symb, file_ptr);
    if ( read_symb != (size_t) num_symb)
    {
        perror ("The following error occurred");
        return NULL;
    }

    return buffer;
}

char* MakeBuffer (const char* filename, long* num_symb)
{
    assert (filename);
    assert (num_symb);

    FILE* file = OpenFile (filename, "rb");
    if ( file == NULL )
        return NULL;

    *num_symb = FileSize (file);
    if (*num_symb == -1L)
        return NULL;

    char* buffer = ReadInBuffer (file, *num_symb);

    enum Errors err = CloseFile (file);
    if (err != NO_ERROR)
        return NULL;

    return buffer;
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

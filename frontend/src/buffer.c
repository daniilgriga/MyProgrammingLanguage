#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"
#include "buffer.h"
#include "color.h"

char* file_reader (struct Buffer_t* buffer, const char* filename)
{
    assert (buffer);

    FILE* file = fopen (filename, "rb");
    if (file == NULL)
    {
        fprintf (stderr, RED_TEXT("Open error\n"));
        return NULL;
    }

    if (count_symbols (buffer, file) == -1)
        return NULL;

    buffer->buffer_ptr = (char*) calloc( (size_t) (buffer->file_size) + 1, sizeof(char));

    if (buffer->buffer_ptr == NULL)
    {
        fprintf(stderr, "Storage error");
        return NULL;
    }

    size_t read_result = fread (buffer->buffer_ptr, sizeof(char), (size_t) buffer->file_size, file);

    if (read_result != (size_t) buffer->file_size)
    {
        fprintf(stderr, "Read error");
        return NULL;
    }

    fclose(file);

    return buffer->buffer_ptr;
}

int count_symbols (struct Buffer_t* buffer, FILE* file)
{
    if (fseek (file, 0, SEEK_END) != 0)
        return -1;

    buffer->file_size = ftell (file);

    if (buffer->file_size == -1)
        return -1;

    if (fseek (file, 0, SEEK_SET) != 0)
        return -1;

    return 0;
}

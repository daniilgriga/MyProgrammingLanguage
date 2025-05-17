#include <stdio.h>

#include "backend.h"
#include "errors.h"

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf (stderr, "Usage: %s <input.ir> <output.nasm>\n", argv[0]);
        return 1;
    }

    enum Errors error = generate_x86_backend (argv[1], argv[2]);
    if (error != NO_ERROR)
        ERROR_CHECK_RET_STATUS (error);

    return 0;
}

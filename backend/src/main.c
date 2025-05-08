#include <stdio.h>

#include "backend.h"

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf (stderr, "Usage: %s <input.ir> <output.nasm>\n", argv[0]);
        return 1;
    }

    generate_x86_backend (argv[1], argv[2]);

    return 0;
}

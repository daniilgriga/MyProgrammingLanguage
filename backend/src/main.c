#include <stdio.h>
#include <string.h>

#include "backend_nasm.h"
#include "backend_bin.h"
#include "errors.h"
#include "create_elf.h"
#include "file.h"

#define EXE_FILE "backend/build/program"

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf (stderr, "Usage: %s <input.ir> <output.nasm>\n", argv[0]);
        return 1;
    }

    //enum Errors error = generate_x86_nasm (argv[1], argv[2]);
    //if (error != NO_ERROR)
    //    ERROR_CHECK_RET_STATUS (error)

    parse_IR_to_array (argv[1]);
    print_IR_array();

    Elf64_Ehdr header = {};
    memset (&header, 0, sizeof(header));

    filing_header (&header);

    FILE* exe_file = OpenFile (EXE_FILE, "wb");
    if (exe_file == NULL)
        return 1;

    if (fwrite (&header, sizeof(header), 1, exe_file) != 1)
    {
        perror ("Error to write the ELF header");
        fclose (exe_file);
        return 1;
    }

    enum Errors close_err = CloseFile (exe_file);
    if (close_err != NO_ERROR)
        return 1;

    return 0;
}

// ! start elf file gen function !
// !        imitters             !

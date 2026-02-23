#include <stdio.h>

#include "backend_nasm.h"
#include "backend_elf.h"
#include "errors.h"
#include "ir_gen.h"
#include "struct.h"
#include "keywords.h"
#include "tree_io.h"
#include "file.h"

#define EXE_FILE        "backend/build/program"
#define NAME_T_FILENAME "backend/Name_Table.txt"
#define TREE_FILENAME   "backend/AST_tree.txt"
#define IR_FILENAME     "backend/program_beta.ir"
#define NASM_FILENAME   "backend/program_beta.nasm"

int main ()
{
    struct Context_t context = {};

    ctor_keywords (&context);

    int read_error = read_name_table (&context, NAME_T_FILENAME);
    if (read_error != 0)
        return 1;

    struct Buffer_t buffer = {};

    struct Node_t* root = read_tree (&buffer, &context, TREE_FILENAME);
    if (root == NULL)
    {
        fprintf (stderr, "ERROR: tree root is NULL after parsing\n");
        free_context (&context);
        return 1;
    }

    struct IRGenerator_t gen = {};
    initial_ir_generator (&gen);
    bypass (&gen, root, &context);

    dump_ir_to_file (&gen, IR_FILENAME);

    enum Errors error = generate_x86_nasm (&gen, NASM_FILENAME);
    if (error != NO_ERROR)
    {
        destructor (root, &buffer, &context);
        ERROR_MESSAGE (error)
        return (int) error;
    }

    error = generate_elf_binary (&gen, EXE_FILE);
    if (error != NO_ERROR)
    {
        destructor (root, &buffer, &context);
        ERROR_MESSAGE (error)
        return (int) error;
    }

    destructor (root, &buffer, &context);

    return 0;
}

#include <stdio.h>

#include "log.h"
#include "tree.h"
#include "tokens.h"
#include "syntax.h"
#include "buffer.h"
#include "ir_gen.h"

#define PROGRAM "frontend/src/factorial_loop.txt"

int main (int argc, const char* argv[])
{
    const char* LogFileName = (argc >= 2)? argv[1] : "log/TreeGraph.html";

    FILE* LogFile = open_log_file (LogFileName);

    struct  Buffer_t  buffer = {};
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = file_reader (&buffer, PROGRAM);

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    struct Node_t* root = GetGrammar (&context);

    fprintf (stderr, "\nNAME TABLE AFTER GetGrammar:\n");
    name_table_dump (stderr, &context);

    dump_in_log_file (root, &context, "TEST OF PROGRAMM");

    struct IRGenerator_t gen = {};
    initial_ir_generator (&gen);
    bypass (&gen, root, &context);

    save_ir_to_file (&gen, "backend/program.ir");

    delete_sub_tree (root);
    buffer_dtor (&buffer);
    close_log_file (LogFile);

    return 0;
}

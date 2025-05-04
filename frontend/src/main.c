#include <stdio.h>

#include "log.h"
#include "tree.h"
#include "tokens.h"
#include "syntax.h"
#include "buffer.h"
#include "ir_gen.h"

int main (int argc, const char* argv[])
{
    const char* LogFileName = (argc >= 2)? argv[1] : "log/TreeGraph.html";

    FILE* LogFile = open_log_file (LogFileName);

    struct  Buffer_t  buffer = {};
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = file_reader (&buffer, "frontend/src/code_example.txt");

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    struct Node_t* root = GetGrammar (&context);

    fprintf (stderr, "\nNAME TABLE AFTER GetGrammar:\n");
    name_table_dump (stderr, &context);

    dump_in_log_file (root, &context, "TEST OF PROGRAMM");

    struct IRGenerator gen = {};
    initial_ir_generator (&gen);
    bypass (&gen, root->left->right->left, &context);

    for (int i = 0; i < gen.instr_count; i++)
        fprintf (stderr, "%s\n", gen.instructions[i]);

    delete_sub_tree (root);
    buffer_dtor (&buffer);
    close_log_file (LogFile);

    return 0;
}

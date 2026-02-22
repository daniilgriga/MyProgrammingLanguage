#include <stdio.h>

#include "log.h"
#include "tree.h"
#include "tokens.h"
#include "syntax.h"
#include "buffer.h"

int main (int argc, const char* argv[])
{
    if (argc < 2)
    {
        fprintf (stderr, "Usage: %s <program.txt>\n", argv[0]);
        return 1;
    }

    const char* program_file = argv[1];

    FILE* LogFile = open_log_file ("log/TreeGraph.html");

    struct  Buffer_t  buffer = {};
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = file_reader (&buffer, program_file);

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    struct Node_t* root = GetGrammar (&context);

    fprintf (stderr, "\nNAME TABLE AFTER GetGrammar:\n");
    name_table_dump (stderr, &context);

    dump_in_log_file (root, &context, "TEST OF PROGRAMM");

    create_tree_file_for_middle_end (root, &context, "middle_end/AST_tree.txt", 0);
    create_name_table_file_for_middle_end (&context, "middle_end/Name_Table.txt");

    delete_sub_tree (root);
    buffer_dtor (&buffer);
    close_log_file (LogFile);

    return 0;
}

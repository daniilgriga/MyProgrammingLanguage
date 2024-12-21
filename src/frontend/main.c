#include <stdio.h>

#include "log.h"
#include "tree.h"
#include "tokens.h"
#include "syntax.h"
#include "buffer.h"

int main (int argc, const char* argv[])
{
    const char* LogFileName = (argc >= 2)? argv[1] : "../../build/TreeGraph.html";

    FILE* LogFile = open_log_file (LogFileName);

    struct  Buffer_t  buffer = {};
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = file_reader (&buffer, "code_example.txt");

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    struct Node_t* root = GetGrammar (&context);

    fprintf (stderr, "\nNAME TABLE AFTER GetGrammar:\n");
    name_table_dump (&context);

    dump_in_log_file (root, &context, "TEST OF PROGRAMM");

    fprintf (stderr, "\n");
    print_tree_postorder (root, &context);
    fprintf (stderr, "\n");

    delete_sub_tree (root);

    buffer_dtor (&buffer);

    close_log_file ();

    return 0;
}

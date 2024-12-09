#include <stdio.h>

#include "log.h"
#include "tree.h"
#include "tokens.h"
#include "syntax.h"

int main (int argc, const char* argv[])
{
    const char* LogFileName = (argc >= 2)? argv[1] : "../../build/TreeGraph.html";

    FILE* LogFile = open_log_file (LogFileName);

    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = "sin(temp+5*size)/num^age$";

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    struct Node_t* root = GetGrammar (&context);

    dump_in_log_file (root, "TEST OF PROGRAMM");

    delete_sub_tree (root);

    close_log_file ();

    return 0;
}

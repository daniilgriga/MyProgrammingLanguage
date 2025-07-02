#include <stdio.h>

#include "file.h"
#include "tree.h"
#include "log.h"

#define NAME_T_FILENAME "middle_end/Name_Table.txt"
#define TREE_FILENAME "middle_end/AST_tree.txt"

int main ()
{
    struct Context_t context = {};

    ctor_keywords (&context);

    int error = read_name_table (&context, NAME_T_FILENAME);
    if (error != 0)
        return 1;

    name_table_dump (stderr, &context);

    // read tree

    // optimization

    // write tree in backend

    return 0;
}

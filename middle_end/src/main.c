#include <stdio.h>

#include "file.h"
#include "tree.h"
#include "log.h"
#include "simplification.h"

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

    struct Buffer_t buffer = {};

    struct Node_t* root = read_tree (&buffer, &context, TREE_FILENAME);
    if (root == NULL)
    {
        fprintf (stderr, "ERROR: root is NULL\n");
        return 1;
    }

    simplification_of_expression (root, NULL);

    create_tree_file_for_backend (root, &context, "backend/AST_tree.txt", 0);
    create_name_table_file_for_backend (&context, "backend/Name_Table.txt");

    destructor (root, &buffer, &context);

    return 0;
}

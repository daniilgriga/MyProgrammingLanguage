#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "color.h"
#include "tree.h"
#include "tree_io.h"

int write_ast_file (struct Node_t* root, struct Context_t* context, const char* filename, int level)
{
    assert (context);
    assert (filename);

    if (root == NULL)
    {
        fprintf (stderr, RED_TEXT("ERROR: ") "File for middle-end not created - tree root not found\n");
        return 1;
    }

    FILE* file = fopen (filename, "wb");
    if (file == NULL)
    {
        fprintf (stderr, "\n" "Could not find the '%s' to be opened!" "\n", filename);
        return 1;
    }

    print_tree_preorder (root, context, file, level);

    return 0;
}

int write_name_table_file (struct Context_t* context, const char* filename)
{
    assert (context);
    assert (filename);

    FILE* file = fopen (filename, "wb");
    if (file == NULL)
    {
        fprintf (stderr, "\n" "Could not find the '%s' to be opened!" "\n", filename);
        return 1;
    }

    for (int i = context->keywords_offset; i < context->table_size; i++)
        fprintf (file, "\"%.*s\" %d %d %d %d %d %d %d %d \n",
                context->name_table[i].name.length,
                context->name_table[i].name.str_pointer,
                context->name_table[i].name.length,
                context->name_table[i].name.is_keyword,
                context->name_table[i].name.added_status,
                context->name_table[i].name.id_type,
                context->name_table[i].name.host_func,
                context->name_table[i].name.counter_params,
                context->name_table[i].name.counter_locals,
                context->name_table[i].name.offset);

    return 0;
}

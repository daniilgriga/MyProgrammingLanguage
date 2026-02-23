#ifndef TREE_H
#define TREE_H

#include "keywords.h"
#include "tree_io.h"

int write_ast_file (struct Node_t* root, struct Context_t* context, const char* filename, int level);

int write_name_table_file (struct Context_t* context, const char* filename);

#endif // TREE_H

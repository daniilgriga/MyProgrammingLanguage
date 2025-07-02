#ifndef TREE_H
#define TREE_H

#include "struct.h"

struct Node_t
{
    int type;

    double value;

    struct Node_t* left;
    struct Node_t* right;
};

struct Buffer_t
{
    char* buffer_ptr;
    char* current_ptr;

    long file_size;
};

int read_name_table (struct Context_t* context, const char* filename);

int name_table_dump (FILE* file, struct Context_t* context);

int ctor_keywords (struct Context_t* context);

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int is_keyword, int length, int added_status);

int find_name (struct Context_t* context, const char* str, int length);

#endif // TREE_H

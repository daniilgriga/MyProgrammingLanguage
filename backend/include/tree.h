#pragma once

#include "struct.h"
#include "keywords.h"

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

struct Node_t* read_tree (struct Buffer_t* buffer, struct Context_t* context, const char* filename);

int create_tree_file_for_middle_end (struct Node_t* root, struct Context_t* context, const char* filename, int level);

int delete_sub_tree (struct Node_t* node);

int delete_node (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor (struct Node_t* node, struct Buffer_t* buffer, struct Context_t* context);

void free_context (struct Context_t* context);

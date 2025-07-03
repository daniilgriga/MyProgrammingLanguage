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

void skip_spaces (char** ptr);

struct Node_t* read_tree (struct Buffer_t* buffer, struct Context_t* context, const char* filename);

struct Node_t* read_node (int level, struct Buffer_t* buffer, struct Context_t* context);

struct Node_t* new_node ();

void print_tree_preorder (struct Node_t* root, struct Context_t* context, FILE* file, int level);

int create_tree_file_for_middle_end (struct Node_t* root, struct Context_t* context, const char* filename, int level);

int delete_sub_tree (struct Node_t* node);

int delete_node (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor (struct Node_t* node, struct Buffer_t* buffer, struct Context_t* context);

void free_context (struct Context_t* context);

#endif // TREE_H

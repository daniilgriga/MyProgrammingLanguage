#pragma once

#include "tokens.h"

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

struct Node_t* new_node (int type, double value, struct Node_t* node_left, struct Node_t* node_right);

int delete_sub_tree       (struct Node_t* node);

int delete_node (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor  (struct Node_t* node, struct Buffer_t* buffer);

void print_tree_preorder_for_file (struct Node_t* node, FILE* filename);

int make_graph  (struct Node_t* node);

void dump_in_log_file (struct Node_t* node, const char* reason, ...);

void clean_buffer (void);

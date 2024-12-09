//#define DEBUG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "tree.h"
#include "enum.h"
#include "log.h"
#include "color_print.h"

static struct Node_t* GlobalNode = NULL;

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

struct Node_t* new_node (int type, double value, struct Node_t* node_left, struct Node_t* node_right)
{
    struct Node_t* node = (struct Node_t*) calloc (1, sizeof(*node));
    if (node == NULL)
    {
        fprintf (stderr, "node is NULL after creating");
        assert (0);
    }

    node->type  = type;
    node->value = value;

    node->left  = node_left;
    node->right = node_right;

    return node;
}

int delete_sub_tree (struct Node_t* node)
{
    if (node->left)  delete_sub_tree (node->left);
    if (node->right) delete_sub_tree (node->right);

    delete_node (node);

    return 0;
}

int delete_node (struct Node_t* node)
{
    if (node == NULL)
        fprintf (stderr, "IN DELETE: node = NULL\n");

    node->type  = 666;
    node->value = 0;

    node->left  = NULL;
    node->right = NULL;

    free (node);

    return 0;
}

int buffer_dtor (struct Buffer_t* buffer)
{
    buffer->current_ptr = NULL;

    free (buffer->buffer_ptr);

    return 0;
}

int destructor (struct Node_t* node, struct Buffer_t* buffer)
{
    assert (node);
    assert (buffer);

    delete_sub_tree (node);
    buffer_dtor (buffer);
}

int its_func_is_root (struct Node_t* node)
{
    assert (node);
    fprintf (stderr, BLUE_TEXT("IN its_func_is_root ") "node = [%p]\n", node);

    if (node->type == OP && (int) node->value == SIN)
        return 1;

    if (node->type == OP && (int) node->value == COS)
        return 1;

    if (node->type == OP && (int) node->value == LN)
        return 1;

    return 0;
}

void print_tree_preorder_for_file (struct Node_t* node, FILE* filename)
{
    assert (node);
    assert (filename);

    assert (node->type == NUM || node->type == OP || node->type == ID);

    if (node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM)  | value = %g   | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP)   | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#20B2AA\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);
    else if (node->type == ID)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (ID)   | number of name in name table = '%lg' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#F00000\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == ROOT)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (ROOT) | value = '%lg' | { son_node = [%p] } }\"; style = filled; fillcolor = \"#F0FFFF\"];\n",
             node, node, node->type, node->value, node->left);

    if (node->left)
        fprintf (filename, "node%p -> node%p;\n", node, node->left);

    if (node->right)
        fprintf (filename, "node%p -> node%p;\n", node, node->right);

    if (node->left)  print_tree_preorder_for_file (node->left , filename);

    if (node->right) print_tree_preorder_for_file (node->right, filename);
}

int make_graph (struct Node_t* node)
{
    assert (node);

    FILE* graph_file = fopen ("../../build/graph_tree.dot", "wb");
    if (graph_file == NULL)
    {
        fprintf(stderr, "\n" RED_TEXT("ERROR open graph_file") "\n");
        return 1;
    }

    fprintf (graph_file, "digraph\n{\n");

    fprintf (graph_file, "bgcolor=\"transparent\"\n");

    print_tree_preorder_for_file (node, graph_file);
    fprintf (graph_file, "\n");

    fprintf (graph_file, "}");
    fprintf (graph_file, "\n");

    fclose  (graph_file);

    return 0;
}

void dump_in_log_file (struct Node_t* node, const char* reason, ...)
{
    if (node == NULL)
        fprintf (stderr, "got node == NULL in dump, reason = \"%s\"\n", reason);

    make_graph (node);

    va_list args;
    va_start (args, reason);

    write_log_file (reason, args);

    va_end (args);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}

//#define DEBUG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"
#include "tree.h"
#include "enum.h"
#include "tokens.h"
#include "color.h"

#define MAX_WORD 100

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

    return 0;
}

const char* get_name (double enum_value)
{
    switch ( (enum Operations) enum_value)
    {
        case   SPACE: return ""; // for fixing warning
        case     ADD: return "PLUS";
        case     SUB: return "SUB";
        case     DIV: return "DIV";
        case     MUL: return "MUL";
        case     POW: return "^";
        case   OP_BR: return "(";
        case   CL_BR: return ")";
        case OP_F_BR: return "LESSSGO";
        case CL_F_BR: return "STOOPIT";
        case   EQUAL: return "=";
        case      IF: return "FORREAL";
        case   WHILE: return "MONEY";
        case     DEF: return "DEFINITION";
        case    CALL: return "CALL";
        case   COMMA: return ",";
        case    GLUE: return "SHUTUP";
        case FN_GLUE: return "NEXT FUNCTION";
        case     SIN: return "SIN";
        case     COS: return "COS";
        case      LN: return "LN";
        case    SQRT: return "SQRT";
        case    ADVT: return "LETHIMCOOK";

        default:      return "bro, wth...";
    }
}

const char* get_type (int type)
{
    switch (type)
    {
        case NUM:   return "NUM";
        case OP:    return "OP";
        case ID:    return "ID";
        case FUNC:  return "FUNC";
        case PARM:  return "PARM";
        case LOCL:  return "LOCL";
        case ROOT:  return "ROOT";

        default:    return "UNKNOWN";
    }
}

void print_tree_preorder_for_file (struct Node_t* node, struct Context_t* context, FILE* filename)
{
    assert (node);
    assert (filename);

    assert (node->type == NUM || node->type == OP || node->type == ID || node->type == FUNC);

    if (node && node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (NUM)  | value = '' %g '' }\"; style = filled; fillcolor = \"#FFD700\"];\n",
                 node, node->type, node->value);

    else if (node && node->type == OP && (int) node->value == GLUE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#E0E0E0\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == OP && (int) node->value == IF)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#68F29D\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == OP && (int) node->value == WHILE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#DF73DF\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == FUNC && (int) node->value == FN_GLUE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#A0A0A0\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == FUNC && (int) node->value == COMMA)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#FEAADF\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == FUNC && (int) node->value == CALL)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#F069F5\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == FUNC && (int) node->value == DEF)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#755CF7\"];\n",
                 node, node->type, get_name (node->value), node->value);


    else if (node && node->type == FUNC)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %.*s ''  (%lg) }\"; style = filled; fillcolor = \"#2EE31E\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer,
                 node->value);

    else if (node && node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#00FFDD\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node && node->type == ID && context->name_table[(int)node->value].name.id_type == PARM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ID)  | name = '' %.*s '' | number in name table = '' %lg '' | id_type = PARM }\"; style = filled; fillcolor = \"#FF5050\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer, node->value);

    else if (node && node->type == ID && context->name_table[(int)node->value].name.id_type == LOCL)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ID)  | name = '' %.*s '' | number in name table = '' %lg '' | id_type = LOCL }\"; style = filled; fillcolor = \"#FF5050\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer, node->value);

    else if (node && node->type == ROOT)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ROOT) | value = '' %lg '' | { son_node = [%p] } }\"; style = filled; fillcolor = \"#F0FFFF\"];\n",
                 node, node->type, node->value, node->left);

    if (node && node->left)
        fprintf (filename, "node%p -> node%p;\n", node, node->left);

    if (node && node->right)
        fprintf (filename, "node%p -> node%p;\n", node, node->right);

    if (node && node->left)  print_tree_preorder_for_file (node->left , context, filename);

    if (node && node->right) print_tree_preorder_for_file (node->right, context, filename);
}

int make_graph (struct Node_t* node, struct Context_t* context)
{
    assert (node);

    FILE* graph_file = fopen ("log/graph_tree.dot", "wb");
    if (graph_file == NULL)
    {
        fprintf(stderr, "\n" RED_TEXT("ERROR open graph_file") "\n");
        return 1;
    }

    fprintf (graph_file, "digraph\n{\n");

    fprintf (graph_file, "bgcolor=\"transparent\"\n");

    print_tree_preorder_for_file (node, context, graph_file);
    fprintf (graph_file, "\n");

    fprintf (graph_file, "}");
    fprintf (graph_file, "\n");

    fclose  (graph_file);

    return 0;
}

void dump_in_log_file (struct Node_t* node, struct Context_t* context, const char* reason, ...)
{
    if (node == NULL)
        fprintf (stderr, "got node == NULL in dump, reason = \"%s\"\n", reason);

    make_graph (node, context);

    va_list args;
    va_start (args, reason);

    write_log_file (reason, args);

    va_end (args);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}

void print_tree_preorder (struct Node_t* root, struct Context_t* context, FILE* file, int level)
{
    assert (root);
    assert (context);
    assert (file);

    fprintf (file, "%*s{ ", level * 4, "");

    fprintf (file, "%d: \"%g\" ", root->type, root->value);

    if (root->left)
        fprintf (file, "\n");

    if (root->right)
        fprintf (file, "\n");

    if (root->left)  print_tree_preorder (root->left, context, file, level + 1);

    if (root->right) print_tree_preorder (root->right, context, file, level + 1);

    fprintf (file, "%*s} \n", (root->left) ? level * 4 : 0, "");
}

int create_tree_file_for_middle_end (struct Node_t* root, struct Context_t* context, const char* filename, int level)
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

int create_name_table_file_for_middle_end (struct Context_t* context, const char* filename)
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


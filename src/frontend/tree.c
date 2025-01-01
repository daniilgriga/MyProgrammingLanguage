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
#include "color_print.h"

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

//========================== BACKEND (WILL BE PLACED IN A SEPARATE FILE) ==========================//

#define _IMIT(format, ...) fprintf(file, "; %s:%d:" "\n" format, __FILE__, __LINE__, ##__VA_ARGS__)

int print_tree_postorder (FILE* file, struct Node_t* node, struct Context_t* context)
{
    static int count_op = 0;
    count_op += 13;

    if (!node)
        return 1;

    _IMIT ("; ( type = %d, value = %lg\n", node->type, node->value);

    if (node->left  && (int) node->value != EQUAL
                    && (int) node->value != IF
                    && (int) node->value != WHILE
                    && (int) node->value != CALL
                    && (int) node->value != DEF
                    && (int) node->value != COMMA   ) print_tree_postorder (file, node->left, context);

    if (node->right && (int) node->value != EQUAL
                    && (int) node->value != IF
                    && (int) node->value != WHILE
                    && (int) node->value != CALL
                    && (int) node->value != DEF
                    && (int) node->value != COMMA   ) print_tree_postorder (file, node->right, context);

    else if (node->type == FUNC && (int) node->value == CALL)
    {
        _IMIT ("push bx"   "\n");
        _IMIT ("push bx"   "\n");
        _IMIT ("push %d"   "\n", context->name_table[context->curr_host_func].name.counter_locals);
        _IMIT ("add"       "\n");

        print_tree_postorder (file, node->right, context);

        _IMIT ("pop bx"    "\n\n");

        _IMIT ("call %lg:" "\n\n", node->left->value);

        _IMIT ("pop bx"    "\n\n");
    }

    else if (node->type == FUNC && (int) node->value == DEF)
    {
        context->curr_host_func = (int) node->left->left->value;

        _IMIT ("%lg:\n", node->left->left->value);

        print_tree_postorder (file, node->right, context);

        _IMIT ("ret\n");
    }

    else if (node->type == FUNC && (int) node->value == COMMA)
    {
        print_tree_postorder (file, node->left, context);

        _IMIT ("; name = '%.*s'" "\n" "pop [bx + %d]"  "\n",
                        (int)context->name_table[(int)node->left->value].name.length,
                             context->name_table[(int)node->left->value].name.str_pointer,
                             context->name_table[(int)node->left->value].name.offset + context->name_table[context->curr_host_func].name.counter_locals - 1);

        if (node->right != NULL)
            print_tree_postorder (file, node->right, context);
    }

    else if (node->type == NUM)
        _IMIT ("push %lg"    "\n",  node->value);

    else if (node->type == ID)
    {
        _IMIT ("; name = '%.*s'" "\n" "push [bx + %d]"  "\n",
                        (int)context->name_table[(int)node->value].name.length,
                             context->name_table[(int)node->value].name.str_pointer,
                             context->name_table[(int)node->value].name.offset);
    }

    else if (node->type == OP && (int) node->value == ADD)
        _IMIT ("add" "\n");

    else if (node->type == OP && (int) node->value == SUB)
        _IMIT ("sub" "\n");

    else if (node->type == OP && (int) node->value == MUL)
        _IMIT ("mul" "\n");

    else if (node->type == OP && (int) node->value == DIV)
        _IMIT ("div" "\n");

    else if (node->type == OP && (int) node->value == EQUAL)
    {
        if (node->right != NULL)
            print_tree_postorder (file, node->right, context);

        _IMIT ("; name = '%.*s'" "\n" "pop  [bx + %d]"  "\n",
                        (int)context->name_table[(int)node->left->value].name.length,
                             context->name_table[(int)node->left->value].name.str_pointer,
                             context->name_table[(int)node->left->value].name.offset);
    }

    else if (node->type == OP && (int) node->value == IF)
    {
        _IMIT ("; START 'IF'. COMPILING LEFT" "\n");

        int old_count_op = count_op;

        print_tree_postorder (file, node->left, context);

        _IMIT ("; 'IF'. TESTING LEFT" "\n");

        _IMIT ("push 0" "\n");

        _IMIT ("je %d:" "\n", old_count_op);

        _IMIT ("; 'IF'. COMPILING RIGHT" "\n");

        print_tree_postorder (file, node->right, context);

        _IMIT ("; END 'IF'. TESTING RIGHT" "\n");

        _IMIT ("%d:"    "\n", old_count_op);
    }

    else if (node->type == OP && (int) node->value == WHILE)
    {
        _IMIT ("; START 'WHILE'. COMPILING LEFT" "\n");
        int old_count_op = count_op;
        _IMIT ("%d:" "\n", old_count_op + 1);

        print_tree_postorder (file, node->left, context);

        _IMIT ("push 0" "\n");

        _IMIT ("; 'WHILE'. TESTING LEFT" "\n");

        _IMIT ("je %d:" "\n", old_count_op);

        _IMIT ("; 'WHILE'. COMPILING RIGHT" "\n");

        print_tree_postorder (file, node->right, context);

        _IMIT ("; END 'WHILE'. TESTING RIGHT" "\n");

        _IMIT ("jmp %d:" "\n", old_count_op + 1);

        _IMIT ("%d:"    "\n", old_count_op);
    }

    else if (node->type == OP && (int) node->value == GLUE)
        _IMIT ("; NOP"  "\n");

    else
        _IMIT ("; %lg"    "\n",  node->value);

    _IMIT ("; ) type = %d, value = %lg\n", node->type, node->value);

    return 0;
}

#undef _IMIT

int print_in_asm_file (const char* filename, struct Node_t* node, struct Context_t* context)
{
    FILE* asm_code = open_log_file (filename);

    name_table_dump (asm_code, context);

    fprintf (asm_code, "push 5" "\n");
    fprintf (asm_code, "pop bx" "\n" "; bx = 5" "\n\n");

    print_tree_postorder (asm_code, node, context);

    close_log_file (asm_code);

    return 0;
}

//========================== BACKEND (WILL BE PLACED IN A SEPARATE FILE) ==========================//

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

void print_tree_preorder_for_file (struct Node_t* node, struct Context_t* context, FILE* filename)
{
    assert (node);
    assert (filename);

    assert (node->type == NUM || node->type == OP || node->type == ID || node->type == FUNC);

    if (node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (NUM)  | value = '' %g '' }\"; style = filled; fillcolor = \"#FFD700\"];\n",
                 node, node->type, node->value);

    else if (node->type == OP && (int) node->value == GLUE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#E0E0E0\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == OP && (int) node->value == IF)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#68F29D\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == OP && (int) node->value == WHILE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#DF73DF\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == FUNC && (int) node->value == FN_GLUE)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#A0A0A0\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == FUNC && (int) node->value == COMMA)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#FEAADF\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == FUNC && (int) node->value == CALL)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#F069F5\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == FUNC && (int) node->value == DEF)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#755CF7\"];\n",
                 node, node->type, get_name (node->value), node->value);


    else if (node->type == FUNC)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (FUNC) | value = '' %.*s ''  (%lg) }\"; style = filled; fillcolor = \"#2EE31E\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer,
                 node->value);

    else if (node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (OP)   | value = '' %s ''  (%lg) }\"; style = filled; fillcolor = \"#00FFDD\"];\n",
                 node, node->type, get_name (node->value), node->value);

    else if (node->type == ID && context->name_table[(int)node->value].name.id_type == PARM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ID)  | name = '' %.*s '' | number in name table = '' %lg '' | id_type = PARM }\"; style = filled; fillcolor = \"#FF5050\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer, node->value);

    else if (node->type == ID && context->name_table[(int)node->value].name.id_type == LOCL)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ID)  | name = '' %.*s '' | number in name table = '' %lg '' | id_type = LOCL }\"; style = filled; fillcolor = \"#FF5050\"];\n",
                 node, node->type,
                 (int) context->name_table[(int)node->value].name.length,
                       context->name_table[(int)node->value].name.str_pointer, node->value);

    else if (node->type == ROOT)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { type = %d (ROOT) | value = '' %lg '' | { son_node = [%p] } }\"; style = filled; fillcolor = \"#F0FFFF\"];\n",
                 node, node->type, node->value, node->left);

    if (node->left)
        fprintf (filename, "node%p -> node%p;\n", node, node->left);

    if (node->right)
        fprintf (filename, "node%p -> node%p;\n", node, node->right);

    if (node->left)  print_tree_preorder_for_file (node->left , context, filename);

    if (node->right) print_tree_preorder_for_file (node->right, context, filename);
}

int make_graph (struct Node_t* node, struct Context_t* context)
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

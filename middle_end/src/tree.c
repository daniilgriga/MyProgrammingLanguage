#define DEBUG_READER

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "assert.h"
#include "color.h"
#include "file.h"
#include "tree.h"

#ifdef DEBUG_READER
    #define ON_DEBUG(...) __VA_ARGS__
#else
    #define ON_DEBUG(...)
#endif

int read_name_table (struct Context_t* context, const char* filename)
{
    long num_symb = 0;
    char* buffer = MakeBuffer (filename, &num_symb);
    if (buffer == NULL)
        return 1;

    char* current = buffer;
    int size = context->table_size;

    while (*current)
    {
        if (*current != '\"')
            break;

        char name[MAX_NAME_LENGTH] = {};
        int length = 0;
        int is_keyword = 0;
        int added_status = 0;
        int id_type = 0;
        int host_func = 0;
        int counter_params = 0;
        int counter_locals = 0;
        int offset = 0;
        int offset_read = 0;

        int items_read = sscanf (current, " \"%50[^\"]\" %d %d %d %d %d %d %d %d %n",
                                name,
                                &length,
                                &is_keyword,
                                &added_status,
                                &id_type,
                                &host_func,
                                &counter_params,
                                &counter_locals, &offset, &offset_read);

        if (items_read != 9)
        {
            fprintf (stderr, "ERROR: not corrected format in \"%s\"\n", filename);
            free (buffer);

            return 1;
        }

        char* name_copy = strdup (name);
        if (!name_copy)
        {
            fprintf (stderr, "ERROR: could not allocated memory for word\n");
            free (buffer);

            return 1;
        }

        context->name_table[size].name.str_pointer = name_copy;
        context->name_table[size].name.length = length;
        context->name_table[size].name.is_keyword = is_keyword;
        context->name_table[size].name.added_status = added_status;
        context->name_table[size].name.id_type = id_type;
        context->name_table[size].name.host_func = host_func;
        context->name_table[size].name.counter_params = counter_params;
        context->name_table[size].name.counter_locals = counter_locals;
        context->name_table[size].name.offset = offset;
        context->name_table[size].name.code = 1; // for dump

        size++;
        current += offset_read;
    }

    context->table_size = size;
    free (buffer);

    return 0;
}

int name_table_dump (FILE* file, struct Context_t* context)
{
    if (context == NULL)
    {
        fprintf (file, "context is NULL\n");
        return 1;
    }

    int j = 0;
    char* str = "";

    if (file != stderr)
    {
        str = ";";

        int i = 0;
        while (i < context->table_size)
        {
            if (context->name_table[i].name.code == SPACE)
                break;

            i++;
        }

        j = i + 1;
    }

    while ( context->name_table[j].name.str_pointer != NULL)
    {
        if (context->name_table[j].name.code == SPACE)
            fprintf (file,  "\n" "%s" YELLOW_TEXT("[%.2d]: ADDRESS = [%p], name = '%.*s'") "\n\n",
                            str, j, context[j].name_table,
                              (int) context->name_table[j].name.length,
                                    context->name_table[j].name.str_pointer);
        else
        {
            if (file == stderr)
                fprintf (file,  "%s" "[%.2d]: " "ADDRESS = [%p], name = '%.*s', length = %d, is_keyword = %d, added_status = %d"
                                "\n" "%s" "id_type = %d, host_func = %d, counter_params = %d, counter_locals = %d, offset = %d\n\n",
                                str, j, context[j].name_table,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.str_pointer,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.is_keyword,
                                        context->name_table[j].name.added_status,
                                str,    context->name_table[j].name.id_type,
                                        context->name_table[j].name.host_func,
                                        context->name_table[j].name.counter_params,
                                        context->name_table[j].name.counter_locals,
                                        context->name_table[j].name.offset);
            else // dump for asm file
                fprintf (file,  "%s" "[%.2d]: " "ADDRESS = [%p], name = '%.*s', %*s lngth = %d, keywrd = %d, added_stts = %d "
                                "id_type = %d, host_fnc = %02d, cntr_prms = %d, cntr_lcls = %d, offset = %d\n",
                                str, j, context[j].name_table,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.str_pointer,
                                    5 - context->name_table[j].name.length, "",
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.is_keyword,
                                        context->name_table[j].name.added_status,
                                        context->name_table[j].name.id_type,
                                        context->name_table[j].name.host_func,
                                        context->name_table[j].name.counter_params,
                                        context->name_table[j].name.counter_locals,
                                        context->name_table[j].name.offset);
        }

        j++;
    }

    return 0;
}

int ctor_keywords (struct Context_t* context)
{
    context->table_size = 0;

    add_struct_in_keywords (context,                 "sin",   SIN  , 1, strlen (                "sin"), 0);
    add_struct_in_keywords (context,                 "cos",   COS  , 1, strlen (                "cos"), 0);
    add_struct_in_keywords (context,                  "ln",    LN  , 1, strlen (                 "ln"), 0);
    add_struct_in_keywords (context,                   "+",   ADD  , 1, strlen (                  "+"), 0);
    add_struct_in_keywords (context,                   "-",   SUB  , 1, strlen (                  "-"), 0);
    add_struct_in_keywords (context,                   "*",   MUL  , 1, strlen (                  "*"), 0);
    add_struct_in_keywords (context,                   "/",   DIV  , 1, strlen (                  "/"), 0);
    add_struct_in_keywords (context,                   "^",   POW  , 1, strlen (                  "^"), 0);
    add_struct_in_keywords (context,                   "(",  OP_BR , 1, strlen (                  "("), 0);
    add_struct_in_keywords (context,                   ")",  CL_BR , 1, strlen (                  ")"), 0);
    add_struct_in_keywords (context,             "lesssgo", OP_F_BR, 1, strlen (            "lesssgo"), 0);
    add_struct_in_keywords (context,             "stoopit", CL_F_BR, 1, strlen (            "stoopit"), 0);
    add_struct_in_keywords (context,                  "is",  EQUAL , 1, strlen (                 "is"), 0);
    add_struct_in_keywords (context,             "forreal",   IF   , 1, strlen (            "forreal"), 0);
    add_struct_in_keywords (context,               "money",  WHILE , 1, strlen (              "money"), 0);
    add_struct_in_keywords (context,          "lethimcook",  ADVT  , 1, strlen (         "lethimcook"), 0);
    add_struct_in_keywords (context,                   ",",  COMMA , 1, strlen (                  ","), 0);
    add_struct_in_keywords (context,              "shutup",  GLUE  , 1, strlen (             "shutup"), 0);
    add_struct_in_keywords (context, "SPACE_FOR_ADDED_OBJ",  SPACE , 1, strlen ("SPACE_FOR_ADDED_OBJ"), 0);

    // comparison operators
    add_struct_in_keywords (context,     "fr",  GT , 1, strlen (    "fr"), 0);
    add_struct_in_keywords (context, "lowkey",  LT , 1, strlen ("lowkey"), 0);
    add_struct_in_keywords (context,  "nocap",  GTE, 1, strlen ( "nocap"), 0);
    add_struct_in_keywords (context,    "nah",  NEQ, 1, strlen (   "nah"), 0);
    add_struct_in_keywords (context, "sameAs",  EQ , 1, strlen ("sameAs"), 0);

    // built-in functions: registered as IDs with added_status=1
    add_struct_in_keywords (context,  "printf",  (enum Operations) ID, 0, strlen ( "printf"), 1);
    add_struct_in_keywords (context,   "scanf",  (enum Operations) ID, 0, strlen (  "scanf"), 1);
    add_struct_in_keywords (context,    "sqrt",  (enum Operations) ID, 0, strlen (   "sqrt"), 1);

    context->keywords_offset = context->table_size;

    return 0;
}

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int is_keyword, int length, int added_status)
{
    if (find_name (context, str, length) != -1)
    {
        fprintf (stderr, "ERROR: could not find \"%.*s\" in name table\n", length, str);
        exit(1);
    }

    context->name_table[context->table_size].name.str_pointer  = str;
    context->name_table[context->table_size].name.code         = code;
    context->name_table[context->table_size].name.is_keyword   = is_keyword;    // YES = 1;
    context->name_table[context->table_size].name.length       = length;
    context->name_table[context->table_size].name.added_status = added_status;

    context->table_size++;

    return 0;
}

int find_name (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if ( strncmp (str, context->name_table[i].name.str_pointer, (size_t) length) == 0 )
            return context->name_table[i].name.code;

    return -1;
}

void skip_spaces (char** ptr)
{
    while (isspace((unsigned char)**ptr))
        (*ptr)++;
}

struct Node_t* read_tree (struct Buffer_t* buffer, struct Context_t* context, const char* filename)
{
    assert (filename && "Filename is NULL\n");
    assert (buffer && "Buffer is NULL\n");

    ON_DEBUG ( fprintf (stderr, "Starting read_tree. Filename = %s\n", filename); )

    FILE* file = OpenFile (filename, "rb");
    if (file == NULL)
        return NULL;

    struct stat st = {};
    if (fstat (fileno (file), &st) != 0)
    {
        fprintf (stderr, "Failed to get file stats for %s. Return NULL.\n", filename);
        CloseFile (file);
        return NULL;
    }

    long file_size = st.st_size;

    buffer->buffer_ptr = (char*) calloc ( (size_t) file_size + 1, sizeof(buffer->buffer_ptr[0]));
    if (buffer->buffer_ptr == NULL)
    {
        fprintf ( stderr, "Could not allocate buffer for file %s. Return NULL.\n", filename);
        CloseFile (file);
        return NULL;
    }

    size_t count = fread (buffer->buffer_ptr, sizeof(buffer->buffer_ptr[0]), (size_t) file_size, file);
    if ((long) count != file_size)
    {
        fprintf (stderr, "Read error: expected %ld bytes, but got %zu\n", file_size, count);
        free (buffer->buffer_ptr);
        CloseFile (file);
        return NULL;
    }

    enum Errors err = CloseFile (file);
    if (err != NO_ERROR)
    {
        free (buffer->buffer_ptr);
        return NULL;
    }

    buffer->current_ptr = buffer->buffer_ptr;
    buffer->file_size = file_size;

    ON_DEBUG ( fprintf ( stderr, "Successfully read file %s. Starting node parsing.\n", filename); )

    return read_node (0, buffer, context);
}

#define INDENT fprintf (stderr, "%*s", level*2, "")

struct Node_t* read_node (int level, struct Buffer_t* buffer, struct Context_t* context)
{
    assert (buffer && "Buffer is NULL in read_node()\n");
    assert (level == 0 ? parent == NULL : parent != NULL);

    skip_spaces (&buffer->current_ptr);

    ON_DEBUG ( fprintf (stderr, "\n"); )
    ON_DEBUG ( INDENT; fprintf (stderr, YELLOW_TEXT("Starting read_node(). Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
               buffer->current_ptr,  buffer->current_ptr, buffer->buffer_ptr); )

    int offset = 0;
    if (sscanf(buffer->current_ptr, " { %n", &offset) != 0 || offset <= 0)
    {
        fprintf (stderr, "No '{' found: offset = %d. Return NULL.\n", offset);
        return NULL;
    }

    buffer->current_ptr += offset;

    ON_DEBUG ( INDENT; fprintf (stderr, GREEN_TEXT("Got an '{'. Creating a node. Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
               buffer->current_ptr,  buffer->current_ptr, buffer->buffer_ptr); )

    struct Node_t* node = new_node ();

    int type = 0;
    char value_str[MAX_NAME_LENGTH] = {};
    offset = 0;

    if (sscanf (buffer->current_ptr, " %d: \"%[^\"]\"%n", &type, value_str, &offset) != 2 || offset <= 0)
    {
        free (node);
        fprintf (stderr, "Failed to parse type and value. Return NULL.\n");
        return NULL;
    }

    buffer->current_ptr += offset;
    node->type = type;

    ON_DEBUG ( INDENT; fprintf (stderr, LIGHT_BLUE_TEXT("Shifted CURRENT_PTR: type = '%d'. Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
               node->type, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    switch (type)
    {
        case NUM:
            node->value = atof (value_str);
            ON_DEBUG ( INDENT; fprintf (stderr, "Parsed NUM: value = %g.\n", node->value); )
            break;
        case OP:
            node->value = (double) atoi (value_str);
            ON_DEBUG ( INDENT; fprintf (stderr, "Parsed OP: value = %g.\n", node->value); )
            break;
        case ID:
        {
            int index = atoi (value_str);

            if (index >= 0 && index < context->table_size)
            {
                node->value = (double) index;
                ON_DEBUG ( INDENT; fprintf (stderr, "Parsed ID: index = %d, value = %g.\n", index, node->value); )
            }
            else
            {
                free (node);
                fprintf (stderr, "Invalid ID index: %d. Return NULL.\n", index);
                return NULL;
            }

            break;
        }

        default:
            node->value = atoi(value_str);
            ON_DEBUG ( INDENT; fprintf (stderr, "Parsed default: value = %g.\n", node->value); )
            break;
    }

    skip_spaces (&buffer->current_ptr);

    if (*buffer->current_ptr == '}')
    {
        buffer->current_ptr++;

        ON_DEBUG ( INDENT; fprintf (stderr, PURPLE_TEXT("Got a '}', SHORT Node END (data = '%g'). Return node. Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
                   node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    node->left = read_node (level + 1, buffer, context);
    if (node->left == NULL)
    {
        fprintf (stderr, "Left subtree is NULL. Return NULL.\n");

        free (node);
        return NULL;
    }

    skip_spaces (&buffer->current_ptr);

    if (*buffer->current_ptr == '}')
    {
        buffer->current_ptr++;

        ON_DEBUG ( INDENT; fprintf (stderr, BLUE_TEXT("Got a '}', SINGLE Node END (value = '%g'). Return node. Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
                   node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr);)
        return node;
    }

    node->right = read_node (level + 1, buffer, context);
    if (node->right == NULL)
    {
        fprintf (stderr, "Right subtree is NULL. Return NULL.\n");

        free (node->left);
        free (node);
        return NULL;
    }

    skip_spaces (&buffer->current_ptr);

    if (*buffer->current_ptr != '}')
    {
        fprintf (stderr, "Does NOT get '}'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
                 buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr);

        free (node->left);
        free (node->right);
        free (node);
        return NULL;
    }

    ON_DEBUG ( INDENT; fprintf (stderr, WHITE_TEXT("Got a '}', FULL Node END (value = '%g'). Return node. Cur = <%.40s...>, [%p]. buffer_ptr = [%p]\n"),
               node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    buffer->current_ptr++;

    return node;
}

struct Node_t* new_node ()
{
    struct Node_t* node = (struct Node_t*) calloc (1, sizeof(*node));
    assert (node && "Failed to allocate node");

    node->type = ROOT;  // firstly root, will change in read_node
    node->value = 0;

    node->left = NULL;
    node->right = NULL;

    return node;
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

int create_tree_file_for_backend (struct Node_t* root, struct Context_t* context, const char* filename, int level)
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

int create_name_table_file_for_backend (struct Context_t* context, const char* filename)
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

int delete_sub_tree (struct Node_t* node)
{
    if (node->left)  delete_sub_tree (node->left);
    if (node->right) delete_sub_tree (node->right);

    return delete_node (node);
}

int delete_node (struct Node_t* node)
{
    if (node == NULL)
    {
        fprintf (stderr, "IN DELETE: node = NULL\n");
        return -1;
    }

    free (node);
    return 0;
}

int buffer_dtor (struct Buffer_t* buffer)
{
    buffer->current_ptr = NULL;

    free (buffer->buffer_ptr);

    return 0;
}

int destructor (struct Node_t* node, struct Buffer_t* buffer, struct Context_t* context)
{
    assert (node);
    assert (buffer);

    fprintf (stderr, "\nDestructor starting...\n");

    free_context (context);
    delete_sub_tree (node);
    buffer_dtor (buffer);

    return 0;
}

void free_context (struct Context_t* context)
{
    if (context)
    {
        for (int i = context->keywords_offset; i < context->table_size; i++)
        {
            if (context->name_table[i].name.str_pointer)
            {
                fprintf (stderr, "Freed str_pointer [%p] at index %d in name_table\n", context->name_table[i].name.str_pointer, i);
                uintptr_t str_ptr = (uintptr_t) context->name_table[i].name.str_pointer;
                free ((void*) str_ptr);
                context->name_table[i].name.str_pointer = NULL;
            }
        }

        context->table_size = 0;
        context->keywords_offset = 0;
        context->curr_host_func = 0;
    }
}

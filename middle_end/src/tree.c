// ! #define DEBUG_READER

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

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
        context->name_table[size].name.code = 1; // ?

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
    add_struct_in_keywords (context,                "sqrt",  SQRT  , 1, strlen (               "sqrt"), 0);
    add_struct_in_keywords (context,             "lesssgo", OP_F_BR, 1, strlen (            "lesssgo"), 0);
    add_struct_in_keywords (context,             "stoopit", CL_F_BR, 1, strlen (            "stoopit"), 0);
    add_struct_in_keywords (context,                  "is",  EQUAL , 1, strlen (                 "is"), 0);
    add_struct_in_keywords (context,             "forreal",   IF   , 1, strlen (            "forreal"), 0);
    add_struct_in_keywords (context,               "money",  WHILE , 1, strlen (              "money"), 0);
    add_struct_in_keywords (context,          "lethimcook",  ADVT  , 1, strlen (         "lethimcook"), 0);
    add_struct_in_keywords (context,                   ",",  COMMA , 1, strlen (                  ","), 0);
    add_struct_in_keywords (context,              "shutup",  GLUE  , 1, strlen (             "shutup"), 0);
    add_struct_in_keywords (context, "SPACE_FOR_ADDED_OBJ",  SPACE , 1, strlen ("SPACE_FOR_ADDED_OBJ"), 0);

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

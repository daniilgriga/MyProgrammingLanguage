#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keywords.h"

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

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code,
                             int is_keyword, int length, int added_status)
{
    if (find_name (context, str, length) != -1)
    {
        fprintf (stderr, "ERROR: keyword \"%.*s\" already exists in name table\n", length, str);
        exit(1);
    }

    context->name_table[context->table_size].name.str_pointer  = str;
    context->name_table[context->table_size].name.code         = code;
    context->name_table[context->table_size].name.is_keyword   = is_keyword;
    context->name_table[context->table_size].name.length       = length;
    context->name_table[context->table_size].name.added_status = added_status;

    context->table_size++;

    return 0;
}

int find_name (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if (context->name_table[i].name.length == length &&
            strncmp (str, context->name_table[i].name.str_pointer, (size_t) length) == 0)
            return context->name_table[i].name.code;

    return -1;
}

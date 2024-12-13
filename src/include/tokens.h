#pragma once

#define MAX_SIZE           100
#define MAX_SIZE_OPERATOR  20

#include "enum.h"

struct Token_t
{
    int type;

    const char* str;
    int         length;
    double      value;

    struct Token_t* left;
    struct Token_t* right;
};

struct Name_t
{
    const char* str_pointer;
    size_t length;

    enum Operations code;
    int is_keyword;

    int added;
};

struct NameTable_t
{
    struct Name_t name;
    int size;
};

struct Context_t
{
    struct  NameTable_t name_table[MAX_SIZE];

    int table_size;

    struct Token_t           token[MAX_SIZE];
};

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int length, int add_status);

int ctor_keywords (struct Context_t* context);

int name_table_dump (struct Context_t* context);

int tokenization (struct Context_t* context, const char* string);

int check_keyword (struct Context_t* context, const char* str, int length);

int find_number_of_keyword (struct Context_t* context, const char* str, int length);

int tokens_dump (struct Context_t* context, int old_count);

int skip_spaces (const char* string, int length, int current_i);

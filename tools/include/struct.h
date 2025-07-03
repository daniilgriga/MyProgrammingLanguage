#ifndef STRUCT_H
#define STRUCT_H

#include "enum.h"

#define MAX_SIZE           1000
#define MAX_SIZE_OPERATOR  200
#define MAX_NAME_LENGTH    100

struct Token_t
{
    int type;

    const char* str;
    int length;
    double value;

    struct Token_t* left;
    struct Token_t* right;
};

struct Name_t
{
    const char* str_pointer;
    int length;

    enum Operations code;
    int is_keyword;

    int added_status;

    int id_type;
    int host_func;
    int counter_params;
    int counter_locals;

    int offset;
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
    int keywords_offset;

    struct Token_t token[MAX_SIZE];

    int curr_host_func;
};

#endif // STRUCT_H

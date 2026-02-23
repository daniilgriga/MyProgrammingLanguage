#pragma once

#include "struct.h"
#include "keywords.h"

enum Errors
{
    NOT_FIND_END_OF_FILE            =   1,
    NOT_FIND_GLUE_MARK              =   2,
    UNDECLARED                      =   4,
    NOT_FIND_OPEN_BRACE             =   8,
    NOT_FIND_CLOSE_BRACE            =  16,
    NOT_FIND_OPEN_BRACE_OF_COND_OP  =  32,
    NOT_FIND_CLOSE_BRACE_OF_COND_OP =  64,
    NOT_FIND_MAIN_FUNC              = 128,
    THIS_NAME_EXIST                 = 256
};

int name_table_dump (FILE* file, struct Context_t* context);

int tokenization (struct Context_t* context, const char* string);

int find_number_of_keyword (struct Context_t* context, const char* str, int length);

int tokens_dump (struct Context_t* context);

int skip_spaces (const char* string, int length, int current_i);

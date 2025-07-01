#pragma once

#include "enum.h"
#include "struct.h"

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int is_keyword, int length, int added_status);

int ctor_keywords (struct Context_t* context);

int name_table_dump (FILE* file, struct Context_t* context);

int tokenization (struct Context_t* context, const char* string);

int find_name (struct Context_t* context, const char* str, int length);

int find_number_of_keyword (struct Context_t* context, const char* str, int length);

int tokens_dump (struct Context_t* context);

int skip_spaces (const char* string, int length, int current_i);

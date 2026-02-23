#pragma once

#include "struct.h"

int ctor_keywords         (struct Context_t* context);

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code,
                             int is_keyword, int length, int added_status);

int find_name              (struct Context_t* context, const char* str, int length);

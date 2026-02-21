#pragma once

#include "errors.h"
#include "ir_gen.h"

enum Errors generate_elf_binary (struct IRGenerator_t* gen, const char* output_filename);

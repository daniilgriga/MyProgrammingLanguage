#pragma once

#include "errors.h"
#include "ir_gen.h"

enum Errors generate_x86_nasm (struct IRGenerator_t* gen, const char* asm_filename);

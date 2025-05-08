#pragma once

#define MAX_LINE 256
#define MAX_TOKENS 10
#define MAX_REGISTERS 14
#define MAX_VARIABLES 50

void generate_x86_backend (const char* ir_filename, const char* asm_filename);

#pragma once

#include "tree.h"
#include "enum.h"

#define MAX_VAR_NAME   32
#define MAX_IR_INSTR  256
#define MAX_INSTR_LEN 128
#define MAX_SYMBOLS    32

struct Symbol
{
    char name[MAX_VAR_NAME];
    char reg[MAX_VAR_NAME];
};

struct IRGenerator
{
    struct Symbol symbols[MAX_SYMBOLS];
    int symbol_count;
    char instructions[MAX_IR_INSTR][MAX_INSTR_LEN];
    int instr_count;
    int reg_count;
};

void initial_ir_generator (struct IRGenerator* gen);

void new_register (struct IRGenerator* gen, char* buffer, size_t size);

char* get_or_add_symbol (struct IRGenerator* gen, const char* name, int length);

void add_symbol_with_reg (struct IRGenerator* gen, const char* name, const char* reg);

void add_instruction (struct IRGenerator* gen, const char* instr);

char* bypass (struct IRGenerator* gen, struct Node_t* node, struct Context_t* context);

int save_ir_to_file (struct IRGenerator* gen, const char* filename);

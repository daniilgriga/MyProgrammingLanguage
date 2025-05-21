#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

struct CodeBuffer_t
{
    FILE* file;
    uint64_t base_addr;
    uint64_t offset;
    uint8_t* buffer;
    size_t buffer_size;
};

enum Errors code_buffer_init (struct CodeBuffer_t* cb, FILE* file, uint64_t base_addr);
void code_buffer_free (struct CodeBuffer_t* cb);
enum Errors code_buffer_write (struct CodeBuffer_t* cb, const uint8_t* bytes, size_t len);

void emit_mov_reg_imm (struct CodeBuffer_t* cb, int reg_num, int64_t imm);
void emit_mov_mem_reg (struct CodeBuffer_t* cb, uint32_t addr, int reg_num);
void emit_mov_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg);
void emit_imul_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg);
void emit_sub_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg);
void emit_add_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg);
void emit_cmp_jle (struct CodeBuffer_t* cb, int reg_num, int32_t imm, uint64_t target_addr);
void emit_jmp (struct CodeBuffer_t* cb, uint64_t target_addr);
void emit_ret (struct CodeBuffer_t* cb);

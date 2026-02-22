#pragma once

#include <stdint.h>
#include <stddef.h>

// emits x86-64 instructions to machine code (bytecode)

// x86-64 registers
typedef enum
{
    RAX = 0, RCX = 1, RDX = 2, RBX = 3,
    RSP = 4, RBP = 5, RSI = 6, RDI = 7,
    R8  = 8, R9  = 9, R10 = 10, R11 = 11,
    R12 = 12, R13 = 13, R14 = 14, R15 = 15
} Register;

// buffer for machine code generation
struct CodeBuffer
{
    uint8_t* data;          // bytecode buffer
    size_t size;            // current size
    size_t capacity;        // buffer capacity
};

struct CodeBuffer* create_code_buffer (size_t initial_capacity);
void destroy_code_buffer (struct CodeBuffer* buf);
void emit_byte  (struct CodeBuffer* buf, uint8_t byte);
void emit_dword (struct CodeBuffer* buf, uint32_t dword);
void emit_qword (struct CodeBuffer* buf, uint64_t qword);

// ========== MOV INSTRUCTIONS ========== //

void encode_mov_reg_reg     (struct CodeBuffer* buf, Register dst, Register src);      // mov reg, reg
void encode_mov_reg_imm64   (struct CodeBuffer* buf, Register dst, uint64_t imm);      // mov reg, imm64
void encode_mov_reg_imm8    (struct CodeBuffer* buf, Register reg, uint8_t imm);       // mov reg, imm8
void encode_mov_reg_mem     (struct CodeBuffer* buf, Register dst, uint64_t addr);     // mov reg, [abs_addr]
void encode_mov_mem_reg     (struct CodeBuffer* buf, uint64_t addr, Register src);     // mov [abs_addr], reg
void encode_mov_byte_mem_imm (struct CodeBuffer* buf, uint64_t addr, uint8_t imm);     // mov byte [addr], imm8
void encode_mov_bl_mem      (struct CodeBuffer* buf, Register base, int32_t offset);   // mov bl, [base+offset]
void encode_mov_mem_bl      (struct CodeBuffer* buf, uint64_t addr);                   // mov [addr], bl
void encode_mov_mem_dl      (struct CodeBuffer* buf, uint64_t addr);                   // mov [addr], dl

// ========== ARITHMETIC INSTRUCTIONS ========== //

void encode_add_reg_reg  (struct CodeBuffer* buf, Register dst, Register src);      // add reg, reg
void encode_add_reg_imm  (struct CodeBuffer* buf, Register dst, uint8_t imm);       // add reg, imm8
void encode_sub_reg_imm  (struct CodeBuffer* buf, Register dst, uint8_t imm);       // sub reg, imm8
void encode_sub_reg_reg  (struct CodeBuffer* buf, Register dst, Register src);      // sub reg, reg
void encode_imul_reg_imm (struct CodeBuffer* buf, Register dst, int32_t imm);       // imul reg, imm32
void encode_imul_reg_reg (struct CodeBuffer* buf, Register dst, Register src);      // imul reg, reg
void encode_idiv_reg     (struct CodeBuffer* buf, Register divisor);                // idiv reg
void encode_cqo          (struct CodeBuffer* buf);                                  // cqo
void encode_xor_reg_reg  (struct CodeBuffer* buf, Register dst, Register src);      // xor reg, reg
void encode_inc_reg      (struct CodeBuffer* buf, Register reg);                    // inc reg
void encode_dec_reg      (struct CodeBuffer* buf, Register reg);                    // dec reg

// ========== COMPARISON AND JUMPS ========== //

void encode_cmp_reg_imm  (struct CodeBuffer* buf, Register reg, int32_t imm);       // cmp reg, imm32
void encode_cmp_reg_reg  (struct CodeBuffer* buf, Register dst, Register src);      // cmp reg, reg
void encode_test_reg_reg (struct CodeBuffer* buf, Register dst, Register src);      // test reg, reg
void encode_jle_rel32    (struct CodeBuffer* buf, int32_t offset);                  // jle rel32
void encode_jmp_rel32    (struct CodeBuffer* buf, int32_t offset);                  // jmp rel32
void encode_je_rel32     (struct CodeBuffer* buf, int32_t offset);                  // je rel32
void encode_jnz_rel32    (struct CodeBuffer* buf, int32_t offset);                  // jnz rel32
void encode_ja_rel32     (struct CodeBuffer* buf, int32_t offset);                  // ja rel32

// ========== STACK OPERATIONS ========== //

void encode_push_reg (struct CodeBuffer* buf, Register reg);                        // push reg
void encode_pop_reg  (struct CodeBuffer* buf, Register reg);                        // pop reg

// ========== CALLS AND RETURNS ========== //

void encode_call_rel32 (struct CodeBuffer* buf, int32_t offset);                    // call rel32
void encode_ret        (struct CodeBuffer* buf);                                    // ret

// ========== SYSTEM CALLS ========== //

void encode_syscall (struct CodeBuffer* buf);                                       // syscall

// ========== 8-BIT ARITHMETIC (for runtime syscalls) ========== //

void encode_cmp_bl_imm8            (struct CodeBuffer* buf, uint8_t imm);                 // cmp bl, imm8
void encode_sub_bl_imm8            (struct CodeBuffer* buf, uint8_t imm);                 // sub bl, imm8
void encode_add_dl_imm8            (struct CodeBuffer* buf, uint8_t imm);                 // add dl, imm8
void encode_div_reg                (struct CodeBuffer* buf, Register divisor);            // div reg (unsigned)
void encode_mov_byte_reg_ind_imm8  (struct CodeBuffer* buf, Register base, uint8_t imm);  // mov byte [reg], imm8
void encode_mov_reg_ind_dl         (struct CodeBuffer* buf, Register base);               // mov [reg], dl

// ========== UTILITIES ========== //

void patch_rel32 (struct CodeBuffer* buf, size_t offset, int32_t value);            // patch relative offset
size_t get_code_position (struct CodeBuffer* buf);                                  // get current position

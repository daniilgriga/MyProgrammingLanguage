#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "x86_emitter.h"

// =========================== x86-64 ENCODING THEORY =========================== //
//
// x86-64 instruction format:
// [Prefixes] [REX] [Opcode] [ModR/M] [SIB] [Displacement] [Immediate]
//
// REX prefix: 0100WRXB (for 64-bit operations and extended registers)
//   W = 1: 64-bit operand
//   R = extension of ModRM.reg
//   X = extension of SIB.index
//   B = extension of ModRM.rm or SIB.base
//
// ModR/M byte: [mod:2][reg:3][rm:3]
//   mod = 00: [rm]           (indirect)
//   mod = 01: [rm + disp8]   (indirect + 1 byte displacement)
//   mod = 10: [rm + disp32]  (indirect + 4 byte displacement)
//   mod = 11: rm             (direct register)
//
// SIB byte: [scale:2][index:3][base:3] (for complex addressing)
//
// ============================================================================== //

// ========== code buffer management ========== //

struct CodeBuffer* create_code_buffer (size_t initial_capacity)
{
    struct CodeBuffer* buf = (struct CodeBuffer*) calloc (1, sizeof (struct CodeBuffer));
    if (!buf)
    {
        fprintf (stderr, "Error: failed to allocate CodeBuffer\n");
        exit(1);
    }

    buf->data = (uint8_t*) calloc (initial_capacity, sizeof (uint8_t));
    if (!buf->data)
    {
        fprintf (stderr, "Error: failed to allocate code buffer data\n");
        free (buf);
        exit(1);
    }

    buf->size = 0;
    buf->capacity = initial_capacity;
    return buf;
}

void destroy_code_buffer (struct CodeBuffer* buf)
{
    if (buf)
    {
        free (buf->data);
        free (buf);
    }
}

static void ensure_capacity (struct CodeBuffer* buf, size_t additional)
{
    if (buf->size + additional > buf->capacity)
    {
        size_t new_capacity = buf->capacity * 2;
        while (new_capacity < buf->size + additional)
            new_capacity *= 2;

        uint8_t* new_data = (uint8_t*) realloc (buf->data, new_capacity);
        if (!new_data)
        {
            fprintf (stderr, "Error: failed to reallocate code buffer\n");
            exit(1);
        }

        buf->data = new_data;
        buf->capacity = new_capacity;
    }
}

void emit_byte (struct CodeBuffer* buf, uint8_t byte)
{
    ensure_capacity (buf, 1);
    buf->data[buf->size++] = byte;
}

void emit_dword (struct CodeBuffer* buf, uint32_t dword)
{
    ensure_capacity (buf, 4);
    buf->data[buf->size++] = (uint8_t)(dword);
    buf->data[buf->size++] = (uint8_t)(dword >> 8);
    buf->data[buf->size++] = (uint8_t)(dword >> 16);
    buf->data[buf->size++] = (uint8_t)(dword >> 24);
}

void emit_qword (struct CodeBuffer* buf, uint64_t qword)
{
    ensure_capacity (buf, 8);
    for (int i = 0; i < 8; i++)
        buf->data[buf->size++] = (uint8_t)(qword >> (i * 8));
}

size_t get_code_position (struct CodeBuffer* buf)
{
    return buf->size;
}

void patch_rel32 (struct CodeBuffer* buf, size_t offset, int32_t value)
{
    assert (offset + 4 <= buf->size);
    buf->data[offset]     = (uint8_t)(value);
    buf->data[offset + 1] = (uint8_t)(value >> 8);
    buf->data[offset + 2] = (uint8_t)(value >> 16);
    buf->data[offset + 3] = (uint8_t)(value >> 24);
}

// ========== helper functions ========== //

// generate REX prefix
static void emit_rex (struct CodeBuffer* buf, int w, int r, int x, int b)
{
    uint8_t rex = 0x40;  // base REX prefix
    if (w) rex |= 0x08;  // REX.W
    if (r) rex |= 0x04;  // REX.R
    if (x) rex |= 0x02;  // REX.X
    if (b) rex |= 0x01;  // REX.B

    // REX needed only if flags set or extended registers used
    if (w || r || x || b)
        emit_byte (buf, rex);
}

// generate ModR/M byte
static void emit_modrm (struct CodeBuffer* buf, uint8_t mod, uint8_t reg, uint8_t rm)
{
    uint32_t mod_bits = ((uint32_t) (mod & 0x3u)) << 6u;
    uint32_t reg_bits = ((uint32_t) (reg & 0x7u)) << 3u;
    uint32_t rm_bits  =  (uint32_t) (rm  & 0x7u);
    uint8_t modrm = (uint8_t) (mod_bits | reg_bits | rm_bits);
    emit_byte (buf, modrm);
}

// ========== MOV instructions ========== //

// mov reg64, reg64  (48 89 /r)
void encode_mov_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    // REX.W = 1 (64-bit), REX.R = src[3], REX.B = dst[3]
    emit_rex (buf, 1, (src >> 3) & 1, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x89);                      // opcode: MOV r/m64, r64
    emit_modrm (buf, 0b11, src & 7, dst & 7);   // mod=11 (register-direct)
}

// mov reg64, imm64  (48 B8+r imm64)
void encode_mov_reg_imm64 (struct CodeBuffer* buf, Register dst, uint64_t imm)
{
    // REX.W = 1, REX.B = dst[3]
    emit_rex (buf, 1, 0, 0, (dst >> 3) & 1);
    emit_byte (buf, 0xB8 + (dst & 7));          // opcode: MOV r64, imm64
    emit_qword (buf, imm);
}

// mov reg64, [abs_addr]  (48 8B 04 25 [addr32])
void encode_mov_reg_mem (struct CodeBuffer* buf, Register dst, uint64_t addr)
{
    // using SIB-addressing with disp32 for absolute addressing
    // ModR/M: mod=00, reg=dst, rm=100 (indicates SIB)
    // SIB: scale=00, index=100 (none), base=101 (disp32)

    emit_rex (buf, 1, (dst >> 3) & 1, 0, 0);
    emit_byte (buf, 0x8B);                      // opcode: MOV r64, r/m64
    emit_modrm (buf, 0b00, dst & 7, 0b100);     // rm=100 => use SIB
    emit_byte (buf, 0x25);                      // SIB: [disp32] (scale=0, index=none, base=disp32)
    emit_dword (buf, (uint32_t)addr);
}

// mov [abs_addr], reg64  (48 89 04 25 [addr32])
void encode_mov_mem_reg (struct CodeBuffer* buf, uint64_t addr, Register src)
{
    emit_rex (buf, 1, (src >> 3) & 1, 0, 0);
    emit_byte (buf, 0x89);                      // opcode: MOV r/m64, r64
    emit_modrm (buf, 0b00, src & 7, 0b100);     // rm=100 => SIB
    emit_byte (buf, 0x25);                      // SIB: [disp32]
    emit_dword (buf, (uint32_t)addr);
}

// ========== arithmetic instructions ========== //

// add reg64, reg64  (48 01 /r)
void encode_add_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (src >> 3) & 1, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x01);                      // opcode: ADD r/m64, r64
    emit_modrm (buf, 0b11, src & 7, dst & 7);
}

// sub reg64, reg64  (48 29 /r)
void encode_sub_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (src >> 3) & 1, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x29);                      // opcode: SUB r/m64, r64
    emit_modrm (buf, 0b11, src & 7, dst & 7);
}

// imul reg64, reg64  (48 0F AF /r)
void encode_imul_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (dst >> 3) & 1, 0, (src >> 3) & 1);
    emit_byte (buf, 0x0F);                      // two-byte opcode prefix
    emit_byte (buf, 0xAF);                      // opcode: IMUL r64, r/m64
    emit_modrm (buf, 0b11, dst & 7, src & 7);
}

// cqo - Convert Quadword to Octaword (48 99)
// extends sign of RAX into RDX:RAX (needed before idiv)
void encode_cqo (struct CodeBuffer* buf)
{
    emit_rex (buf, 1, 0, 0, 0);
    emit_byte (buf, 0x99);
}

// idiv reg64  (48 F7 /7)
void encode_idiv_reg (struct CodeBuffer* buf, Register divisor)
{
    emit_rex (buf, 1, 0, 0, (divisor >> 3) & 1);
    emit_byte (buf, 0xF7);                      // opcode group
    emit_modrm (buf, 0b11, 7, divisor & 7);     // /7 = IDIV
}

// ========== comparisons and jumps ========== //

// cmp reg64, imm32  (48 81 /7 imm32)
void encode_cmp_reg_imm (struct CodeBuffer* buf, Register reg, int32_t imm)
{
    emit_rex (buf, 1, 0, 0, (reg >> 3) & 1);
    emit_byte (buf, 0x81);                      // opcode: CMP r/m64, imm32
    emit_modrm (buf, 0b11, 7, reg & 7);         // /7 = CMP
    emit_dword (buf, (uint32_t)imm);
}

// jle rel32  (0F 8E rel32)
void encode_jle_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x8E);
    emit_dword (buf, (uint32_t)offset);
}

// jmp rel32  (E9 rel32)
void encode_jmp_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0xE9);
    emit_dword (buf, (uint32_t)offset);
}

// je rel32  (0F 84 rel32)
void encode_je_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x84);
    emit_dword (buf, (uint32_t)offset);
}

// ========== stack operations ========== //

// push reg64  (50+r)
void encode_push_reg (struct CodeBuffer* buf, Register reg)
{
    // if register is extended (R8-R15), REX needed
    if (reg >= R8)
        emit_rex (buf, 0, 0, 0, 1);
    emit_byte (buf, 0x50 + (reg & 7));
}

// pop reg64  (58+r)
void encode_pop_reg (struct CodeBuffer* buf, Register reg)
{
    if (reg >= R8)
        emit_rex (buf, 0, 0, 0, 1);
    emit_byte (buf, 0x58 + (reg & 7));
}

// ========== calls and returns ========== //

// call rel32  (E8 rel32)
void encode_call_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0xE8);
    emit_dword (buf, (uint32_t)offset);
}

// ret  (C3)
void encode_ret (struct CodeBuffer* buf)
{
    emit_byte (buf, 0xC3);
}

// ========== system calls ========== //

// syscall  (0F 05)
void encode_syscall (struct CodeBuffer* buf)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x05);
}

// ========== additional instructions for runtime ========== //

// xor reg, reg  (REX.W 33 /r)
void encode_xor_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (dst >> 3) & 1, 0, (src >> 3) & 1);
    emit_byte (buf, 0x33);
    emit_modrm (buf, 3, dst, src);
}

// inc reg  (REX.W FF /0)
void encode_inc_reg (struct CodeBuffer* buf, Register reg)
{
    emit_rex (buf, 1, 0, 0, (reg >> 3) & 1);
    emit_byte (buf, 0xFF);
    emit_modrm (buf, 3, 0, reg);
}

// dec reg  (REX.W FF /1)
void encode_dec_reg (struct CodeBuffer* buf, Register reg)
{
    emit_rex (buf, 1, 0, 0, (reg >> 3) & 1);
    emit_byte (buf, 0xFF);
    emit_modrm (buf, 3, 1, reg);
}

// neg reg  (REX.W F7 /3) - two's complement negation
void encode_neg_reg (struct CodeBuffer* buf, Register reg)
{
    emit_rex (buf, 1, 0, 0, (reg >> 3) & 1);
    emit_byte (buf, 0xF7);
    emit_modrm (buf, 3, 3, reg & 7);
}

// jns rel32  (0F 89 rel32) - jump if not sign (SF=0, i.e. value >= 0)
void encode_jns_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x89);
    emit_dword (buf, (uint32_t)offset);
}

// test reg, reg  (REX.W 85 /r)
void encode_test_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (src >> 3) & 1, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x85);
    emit_modrm (buf, 3, src, dst);
}

// jnz rel32  (0F 85 rel32)
void encode_jnz_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x85);
    emit_dword (buf, (uint32_t)offset);
}

// ja rel32  (0F 87 rel32) - jump if above (unsigned >)
void encode_ja_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x87);
    emit_dword (buf, (uint32_t)offset);
}

// jl rel32  (0F 8C rel32) - jump if less (signed <)
void encode_jl_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x8C);
    emit_dword (buf, (uint32_t)offset);
}

// jg rel32  (0F 8F rel32) - jump if greater (signed >)
void encode_jg_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x8F);
    emit_dword (buf, (uint32_t)offset);
}

// jge rel32  (0F 8D rel32) - jump if greater or equal (signed >=)
void encode_jge_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x8D);
    emit_dword (buf, (uint32_t)offset);
}

// jne rel32  (0F 85 rel32) - jump if not equal (!=)
void encode_jne_rel32 (struct CodeBuffer* buf, int32_t offset)
{
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x85);
    emit_dword (buf, (uint32_t)offset);
}

// cmp reg, reg  (REX.W 3B /r)
void encode_cmp_reg_reg (struct CodeBuffer* buf, Register dst, Register src)
{
    emit_rex (buf, 1, (dst >> 3) & 1, 0, (src >> 3) & 1);
    emit_byte (buf, 0x3B);
    emit_modrm (buf, 3, dst, src);
}

// add reg, imm8  (REX.W 83 /0 ib)
void encode_add_reg_imm (struct CodeBuffer* buf, Register dst, uint8_t imm)
{
    emit_rex (buf, 1, 0, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x83);
    emit_modrm (buf, 3, 0, dst);
    emit_byte (buf, imm);
}

// sub reg, imm8  (REX.W 83 /5 ib)
void encode_sub_reg_imm (struct CodeBuffer* buf, Register dst, uint8_t imm)
{
    emit_rex (buf, 1, 0, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x83);
    emit_modrm (buf, 3, 5, dst);
    emit_byte (buf, imm);
}

// imul reg, imm32  (REX.W 69 /r id)
void encode_imul_reg_imm (struct CodeBuffer* buf, Register dst, int32_t imm)
{
    emit_rex (buf, 1, (dst >> 3) & 1, 0, (dst >> 3) & 1);
    emit_byte (buf, 0x69);
    emit_modrm (buf, 3, dst, dst);
    emit_dword (buf, (uint32_t)imm);
}

// mov reg, imm8  (only for AL/BL/CL/DL - 8-bit registers)
void encode_mov_reg_imm8 (struct CodeBuffer* buf, Register reg, uint8_t imm)
{
    emit_byte (buf, 0xB0 + (reg & 7));
    emit_byte (buf, imm);
}

// mov byte [addr], imm8  (C6 /0 imm8 with absolute addressing)
void encode_mov_byte_mem_imm (struct CodeBuffer* buf, uint64_t addr, uint8_t imm)
{
    emit_byte (buf, 0xC6);
    emit_byte (buf, 0x04);                      // modrm: mod=00, reg=0, rm=100 (SIB)
    emit_byte (buf, 0x25);                      // sib: scale=00, index=100, base=101 (abs32)
    emit_dword (buf, (uint32_t)addr);
    emit_byte (buf, imm);
}

// mov bl, [base+offset]  (8A /r with disp8 or disp32)
void encode_mov_bl_mem (struct CodeBuffer* buf, Register base, int32_t offset)
{
    emit_byte (buf, 0x8A);                      // opcode for mov r8, r/m8

    if (offset == 0 && base != RBP)
    {
        emit_modrm (buf, 0, RBX, base);         // mod=00, no displacement
    }
    else if (offset >= -128 && offset <= 127)
    {
        emit_modrm (buf, 1, RBX, base);         // mod=01, disp8
        emit_byte (buf, (uint8_t)offset);
    }
    else
    {
        emit_modrm (buf, 2, RBX, base);         // mod=10, disp32
        emit_dword (buf, (uint32_t)offset);
    }
}

// mov [addr], bl  (88 /r with absolute addressing)
void encode_mov_mem_bl (struct CodeBuffer* buf, uint64_t addr)
{
    emit_byte (buf, 0x88);                      // opcode for mov r/m8, r8
    emit_byte (buf, 0x1C);                      // modrm: mod=00, reg=3 (BL), rm=100 (SIB)
    emit_byte (buf, 0x25);                      // sib: scale=00, index=100, base=101 (abs32)
    emit_dword (buf, (uint32_t)addr);
}

// mov [addr], dl  (88 /r with absolute addressing, DL = reg 2)
void encode_mov_mem_dl (struct CodeBuffer* buf, uint64_t addr)
{
    emit_byte (buf, 0x88);                      // opcode for mov r/m8, r8
    emit_byte (buf, 0x14);                      // modrm: mod=00, reg=2 (DL), rm=100 (SIB)
    emit_byte (buf, 0x25);                      // sib: scale=00, index=100, base=101 (abs32)
    emit_dword (buf, (uint32_t)addr);
}

// ========== 8-bit arithmetic for runtime syscalls ========== //

// cmp bl, imm8  (80 FB ib)
void encode_cmp_bl_imm8 (struct CodeBuffer* buf, uint8_t imm)
{
    emit_byte (buf, 0x80);                      // opcode: 8-bit immediate group
    emit_byte (buf, 0xFB);                      // modrm: mod=11, reg=7 (/7=CMP), rm=3 (BL)
    emit_byte (buf, imm);
}

// sub bl, imm8  (80 EB ib)
void encode_sub_bl_imm8 (struct CodeBuffer* buf, uint8_t imm)
{
    emit_byte (buf, 0x80);                      // opcode: 8-bit immediate group
    emit_byte (buf, 0xEB);                      // modrm: mod=11, reg=5 (/5=SUB), rm=3 (BL)
    emit_byte (buf, imm);
}

// add dl, imm8  (80 C2 ib)
void encode_add_dl_imm8 (struct CodeBuffer* buf, uint8_t imm)
{
    emit_byte (buf, 0x80);                      // opcode: 8-bit immediate group
    emit_byte (buf, 0xC2);                      // modrm: mod=11, reg=0 (/0=ADD), rm=2 (DL)
    emit_byte (buf, imm);
}

// div reg64  (48 F7 /6) - unsigned division RDX:RAX / reg
void encode_div_reg (struct CodeBuffer* buf, Register divisor)
{
    emit_rex (buf, 1, 0, 0, (divisor >> 3) & 1);
    emit_byte (buf, 0xF7);                      // opcode group
    emit_modrm (buf, 0b11, 6, divisor & 7);     // /6 = DIV
}

// mov byte [reg], imm8  (C6 /0 with register-indirect addressing)
void encode_mov_byte_reg_ind_imm8 (struct CodeBuffer* buf, Register base, uint8_t imm)
{
    emit_byte (buf, 0xC6);                      // opcode: MOV r/m8, imm8
    emit_modrm (buf, 0b00, 0, base & 7);        // mod=00, reg=0 (/0), rm=base
    emit_byte (buf, imm);
}

// mov [reg], dl  (88 /r with register-indirect addressing)
void encode_mov_reg_ind_dl (struct CodeBuffer* buf, Register base)
{
    emit_byte (buf, 0x88);                      // opcode: MOV r/m8, r8
    emit_modrm (buf, 0b00, RDX & 7, base & 7);  // mod=00, reg=2 (DL), rm=base
}

// cvtsi2sd xmm0, reg64  (F2 REX.W 0F 2A /r)
// converts 64-bit integer in reg to double in xmm0
void encode_cvtsi2sd_xmm0_reg (struct CodeBuffer* buf, Register src)
{
    emit_byte (buf, 0xF2);                      // SSE prefix
    emit_rex  (buf, 1, 0, 0, (src >> 3) & 1);   // REX.W + REX.B if src >= r8
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x2A);                      // cvtsi2sd opcode
    emit_modrm (buf, 0b11, 0, src & 7);         // mod=11, reg=xmm0(0), rm=src
}

// sqrtsd xmm0, xmm0  (F2 0F 51 /r)
// computes square root of double in xmm0, result in xmm0
void encode_sqrtsd_xmm0_xmm0 (struct CodeBuffer* buf)
{
    emit_byte (buf, 0xF2);                      // SSE prefix
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x51);                      // sqrtsd opcode
    emit_modrm (buf, 0b11, 0, 0);               // mod=11, reg=xmm0, rm=xmm0
}

// cvttsd2si reg64, xmm0  (F2 REX.W 0F 2C /r)
// converts double in xmm0 to 64-bit integer in reg (truncate toward zero)
void encode_cvttsd2si_reg_xmm0 (struct CodeBuffer* buf, Register dst)
{
    emit_byte (buf, 0xF2);                      // SSE prefix
    emit_rex  (buf, 1, (dst >> 3) & 1, 0, 0);   // REX.W + REX.R if dst >= r8
    emit_byte (buf, 0x0F);
    emit_byte (buf, 0x2C);                      // cvttsd2si opcode
    emit_modrm (buf, 0b11, dst & 7, 0);         // mod=11, reg=dst, rm=xmm0(0)
}

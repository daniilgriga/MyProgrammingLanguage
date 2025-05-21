#include <stdio.h>

#include "emitters.h"

enum Errors code_buffer_init (struct CodeBuffer_t* cb, FILE* file, uint64_t base_addr)
{
    cb->file = file;
    cb->base_addr = base_addr;
    cb->offset = 0;
    cb->buffer_size = 1024;

    cb->buffer = (uint8_t*) calloc (cb->buffer_size, sizeof (uint8_t));
    if (cb->buffer == NULL)
    {
        fprintf (stderr, "Error: failed to allocate cb buffer\n");
        return CALLOC_ERROR;
    }

    return NO_ERROR;
}

void code_buffer_free (struct CodeBuffer_t* cb)
{
    free (cb->buffer);
    cb->buffer_size = 0;
}

enum Errors code_buffer_write (struct CodeBuffer_t* cb, const uint8_t* bytes, size_t len)
{
    while (cb->offset + len > cb->buffer_size)
    {
        cb->buffer_size *= 2;
        cb->buffer = (uint8_t*) realloc (cb->buffer, cb->buffer_size);
        if (cb->buffer == NULL)
        {
            fprintf (stderr, "Error: failed to realloc code buffer\n");
            return CALLOC_ERROR;
        }
    }

    memcpy (cb->buffer + cb->offset, bytes, len);

    cb->offset += len;

    return NO_ERROR;
}

// ! fprintf in asm file in every emitter ! //

//============================== EMITTERS ZONE START ==============================//

//==================== Emitter for mov reg, imm ====================//
void emit_mov_reg_imm (struct CodeBuffer_t* cb, int reg_num, int64_t imm)
{
    uint8_t rex = 0x48;                                 // REX-prefix: REX.W (64bit operation)
    uint8_t opcode = 0xC7;                              // opcode:    "writing a value to a register"
    uint8_t modrm = (uint8_t) (0xC0 | reg_num);         // ModR/M:    "a byte that specifies which register is used" | '0xC0' - mask
    uint8_t bytes[] = { rex, opcode, modrm };
    code_buffer_write (cb, bytes, 3);
    code_buffer_write (cb, (uint8_t*)&imm, 4);
}

//==================== Emitter for mov [addr], reg ====================//
void emit_mov_mem_reg (struct CodeBuffer_t* cb, uint32_t addr, int reg_num)
{
    uint8_t rex = 0x48;                                 // REX-prefix: REX.W
    uint8_t opcode = 0x89;                              // opcode:     "move the value from the register to the memory"
    uint8_t modrm = (uint8_t) (0x04 | (reg_num << 3));  // ModR/M:     mod=00, reg=reg_num, rm=100 (SIB)
    uint8_t sib = 0x25;                                 // SIB:        scale=00, index=010, base=101 "take address directly from the next 4 bytes"
    uint8_t bytes[] = { rex, opcode, modrm, sib };
    code_buffer_write (cb, bytes, 4);
    code_buffer_write (cb, (uint8_t*)&addr, 4);
}

//==================== Emitter for mov reg, reg ====================//
void emit_mov_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg)
{
    uint8_t rex = 0x48;                                 // REX-prefix: REX.W
    uint8_t opcode = 0x89;                              // opcode:     "move value from register to register"
    uint8_t modrm = (uint8_t) (0xC0 | (src_reg << 3) | dest_reg);
    uint8_t bytes[] = { rex, opcode, modrm };
    code_buffer_write (cb, bytes, 3);
}

//==================== Emitter for imul reg, reg ====================//
void emit_imul_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg)
{
    uint8_t rex = 0x48;                                 // REX.W
    uint8_t opcode[] = { 0x0F, 0xAF };                  // 0x0F - for extended instructions | 0xAF - specific opcode for imul
    uint8_t modrm = (uint8_t) (0xC0 | (dest_reg << 3) | src_reg);
    uint8_t bytes[] = { rex, opcode[0], opcode[1], modrm };
    code_buffer_write (cb, bytes, 4);
}

//==================== Emitter for sub reg, reg ====================//
void emit_sub_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg)
{
    uint8_t rex = 0x48;                                 // REX.W
    uint8_t opcode = 0x2B;                              // opcode: sub when subtracting a register from a register.
    uint8_t modrm = (uint8_t) (0xC0 | (src_reg << 3) | dest_reg);
    uint8_t bytes[] = { rex, opcode, modrm };
    code_buffer_write (cb, bytes, 3);
}

//==================== Emitter for add reg, reg ====================//
void emit_add_reg_reg (struct CodeBuffer_t* cb, int dest_reg, int src_reg)
{
    uint8_t rex = 0x48;                                 // REX.W
    uint8_t opcode = 0x01;                              // opcode: add register to register
    uint8_t modrm = (uint8_t) (0xC0 | (src_reg << 3) | dest_reg);
    uint8_t bytes[] = { rex, opcode, modrm };
    code_buffer_write (cb, bytes, 3);
}

//==================== Emitter for cmp reg, imm && jle ====================//
void emit_cmp_jle (struct CodeBuffer_t* cb, int reg_num, int32_t imm, uint64_t target_addr)
{
    uint8_t rex = 0x48;                                 // REX.W
    uint8_t opcode = 0x81;                              // opcode: cmp with immediate.
    uint8_t modrm = (uint8_t) (0xF8 | reg_num);
    uint8_t bytes[] = { rex, opcode, modrm };
    code_buffer_write (cb, bytes, 3);
    code_buffer_write (cb, (uint8_t*)&imm, 4);

    int64_t rel_offset = (int64_t) (target_addr - (cb->base_addr + cb->offset + 6));
    uint8_t jle_opcode[] = { 0x0F, 0x8E };              // opcode: jle (jump if less than or equal).
    code_buffer_write (cb, jle_opcode, 2);
    code_buffer_write (cb, (uint8_t*)&rel_offset, 4);
}

//==================== Emitter for jmp ====================//
void emit_jmp (struct CodeBuffer_t* cb, uint64_t target_addr)
{
    uint8_t opcode = 0xE9;                              // opcode: jmp with 32-bit relative offset
    int32_t rel_offset = (int32_t) (target_addr - (cb->base_addr + cb->offset + 5));
    code_buffer_write (cb, &opcode, 1);
    code_buffer_write (cb, (uint8_t*)&rel_offset, 4);
}

//==================== Emitter for ret ====================//
void emit_ret (struct CodeBuffer_t* cb)
{
    uint8_t opcode = 0xC3;                              // opcode: ret
    code_buffer_write (cb, &opcode, 1);
}

//============================== EMITTERS ZONE END ==============================//

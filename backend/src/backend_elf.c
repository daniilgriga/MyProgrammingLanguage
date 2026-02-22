#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "backend_elf.h"
#include "elf_builder.h"
#include "x86_emitter.h"
#include "errors.h"

#define MAX_LABELS 100
#define MAX_VARIABLES 100
#define MAX_LENGTH_NAME 32

struct Label
{
    char name[MAX_LENGTH_NAME];
    size_t code_offset;         // offset in .text
    int is_resolved;            // 1 if offset is known
};

struct Variable
{
    char name[MAX_LENGTH_NAME];
    size_t data_offset;         // offset in .data
};

struct PendingPatch
{
    size_t patch_offset;                 // where to patch (in code buffer)
    char target_label[MAX_LENGTH_NAME];  // label to jump to
};

struct CompilerState
{
    struct ElfBuilder* elf;
    struct Label labels[MAX_LABELS];
    int label_count;
    struct Variable variables[MAX_VARIABLES];
    int variable_count;
    char loop_stack[MAX_LABELS][MAX_LENGTH_NAME];   // stack of loop labels
    int loop_stack_depth;
    char if_stack[MAX_LABELS][MAX_LENGTH_NAME];     // stack of if labels
    int if_stack_depth;
    struct PendingPatch patches[MAX_LABELS];        // pending jump patches
    int patch_count;
};

// ========== helper functions ========== //

static int is_number (const char* str)
{
    if (!str || *str == '\0') return 0;
    char* endptr = NULL;
    strtol (str, &endptr, 10);

    return (*endptr == '\0');
}

static int is_register (const char* str)
{
    return (str && str[0] == 'r' && is_number (str + 1));
}

static Register parse_register (const char* reg_str)
{
    if (!is_register (reg_str))
    {
        fprintf (stderr, "Error: invalid register '%s'\n", reg_str);
        exit(1);
    }

    int reg_num = atoi (reg_str + 1);

    // map r0-r13 to x86-64 registers (skip RSP and RBP which are used for stack)
    // r0=rax, r1=rcx, r2=rdx, r3=rbx, r4=rsi, r5=rdi, r6-r13=R8-R15
    static const Register reg_map[] = {
        RAX, RCX, RDX, RBX, RSI, RDI, R8, R9,
        R10, R11, R12, R13, R14, R15
    };

    if (reg_num >= 0 && reg_num < 14)
        return reg_map[reg_num];

    fprintf (stderr, "Error: register number %d out of range (0-13)\n", reg_num);
    exit(1);
}

// ========== variable management ========== //

static struct Variable* find_variable (struct CompilerState* state, const char* name)
{
    for (int i = 0; i < state->variable_count; i++)
        if (strcmp (state->variables[i].name, name) == 0)
            return &state->variables[i];

    return NULL;
}

static struct Variable* add_variable (struct CompilerState* state, const char* name)
{
    struct Variable* var = find_variable (state, name);
    if (var)
        return var;

    if (state->variable_count >= MAX_VARIABLES)
    {
        fprintf (stderr, "Error: too many variables\n");
        exit(1);
    }

    var = &state->variables[state->variable_count++];
    strncpy (var->name, name, MAX_LENGTH_NAME - 1);
    var->name[MAX_LENGTH_NAME - 1] = '\0';

    var->data_offset = get_data_offset (state->elf);
    add_data_qword (state->elf, 0);  // initialize to 0

    return var;
}

// ========== label management ========== //

static struct Label* find_label (struct CompilerState* state, const char* name)
{
    for (int i = 0; i < state->label_count; i++)
        if (strcmp (state->labels[i].name, name) == 0)
            return &state->labels[i];
    return NULL;
}

static struct Label* add_label (struct CompilerState* state, const char* name)
{
    struct Label* label = find_label (state, name);
    if (label) return label;

    if (state->label_count >= MAX_LABELS)
    {
        fprintf (stderr, "Error: too many labels\n");
        exit(1);
    }

    label = &state->labels[state->label_count++];
    strncpy (label->name, name, MAX_LENGTH_NAME - 1);
    label->name[MAX_LENGTH_NAME - 1] = '\0';
    label->code_offset = 0;
    label->is_resolved = 0;

    return label;
}

static void resolve_label (struct CompilerState* state, const char* name, size_t offset)
{
    struct Label* label = find_label (state, name);
    if (!label)
        label = add_label (state, name);

    label->code_offset = offset;
    label->is_resolved = 1;
}

// ========== IR instruction compilation ========== //

static void compile_ir_instruction (struct CompilerState* state, const char* instruction)
{
    assert (state);
    assert (instruction);

    struct CodeBuffer* code = get_text_buffer (state->elf);
    char instr_copy[256] = {};
    strncpy (instr_copy, instruction, sizeof (instr_copy) - 1);

    char* token = strtok (instr_copy, " ,");
    if (!token) return;

    fprintf (stderr, "Compiling: %s\n", instruction);

    // ========== FUNCTION ========== //
    if (strcmp (token, "function") == 0)
    {
        char* func_name = strtok (NULL, " ,");
        if (!func_name) return;

        resolve_label (state, func_name, get_text_offset (state->elf));

        // function prologue
        encode_push_reg (code, RBP);
        encode_mov_reg_reg (code, RBP, RSP);
    }
    // ========== END_FUNCTION ========== //
    else if (strcmp (token, "end_function") == 0)
    {
        // function epilogue
        encode_mov_reg_reg (code, RSP, RBP);
        encode_pop_reg (code, RBP);
        encode_ret (code);
    }
    // ========== SET (MOV) ========== //
    else if (strcmp (token, "set") == 0)
    {
        char* dst = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!dst || !src) return;

        // handle ABI register names (rdi, rsi, etc.) for syscall argument passing
        Register dst_reg;
        int dst_is_reg = 0;

        if (is_register (dst))
        {
            dst_reg = parse_register (dst);
            dst_is_reg = 1;
        }
        else if (strcmp (dst, "rdi") == 0) { dst_reg = RDI; dst_is_reg = 1; }
        else if (strcmp (dst, "rsi") == 0) { dst_reg = RSI; dst_is_reg = 1; }
        else if (strcmp (dst, "rax") == 0) { dst_reg = RAX; dst_is_reg = 1; }

        if (dst_is_reg)
        {
            if (is_register (src))
            {
                Register src_reg = parse_register (src);
                encode_mov_reg_reg (code, dst_reg, src_reg);
            }
            else if (is_number (src))
            {
                uint64_t imm = (uint64_t) atoll (src);
                encode_mov_reg_imm64 (code, dst_reg, imm);
            }
            else  // variable
            {
                struct Variable* var = find_variable (state, src);
                if (var)
                {
                    uint64_t addr = get_data_absolute_addr (state->elf, var->data_offset);
                    encode_mov_reg_mem (code, dst_reg, addr);
                }
            }
        }
    }
    // ========== STORE ========== //
    else if (strcmp (token, "store") == 0)
    {
        char* var_name = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!var_name || !src) return;

        struct Variable* var = add_variable (state, var_name);
        uint64_t addr = get_data_absolute_addr (state->elf, var->data_offset);

        if (is_register (src))
        {
            Register src_reg = parse_register (src);
            encode_mov_mem_reg (code, addr, src_reg);
        }
    }
    // ========== ADD ========== //
    else if (strcmp (token, "add") == 0)
    {
        char* dst = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!dst || !src) return;

        if (is_register (dst) && is_register (src))
        {
            Register dst_reg = parse_register (dst);
            Register src_reg = parse_register (src);
            encode_add_reg_reg (code, dst_reg, src_reg);
        }
    }
    // ========== SUB ========== //
    else if (strcmp (token, "sub") == 0)
    {
        char* dst = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!dst || !src) return;

        if (is_register (dst) && is_register (src))
        {
            Register dst_reg = parse_register (dst);
            Register src_reg = parse_register (src);
            encode_sub_reg_reg (code, dst_reg, src_reg);
        }
    }
    // ========== MUL ========== //
    else if (strcmp (token, "mul") == 0)
    {
        char* dst = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!dst || !src) return;

        if (is_register (dst) && is_register (src))
        {
            Register dst_reg = parse_register (dst);
            Register src_reg = parse_register (src);
            encode_imul_reg_reg (code, dst_reg, src_reg);
        }
    }
    // ========== DIV ========== //
    else if (strcmp (token, "div") == 0)
    {
        char* dst = strtok (NULL, " ,");
        char* src = strtok (NULL, " ,");

        if (!dst || !src) return;

        if (is_register (dst) && is_register (src))
        {
            Register dst_reg = parse_register (dst);
            Register src_reg = parse_register (src);

            // move dividend to RAX, extend to RDX:RAX, divide
            encode_mov_reg_reg (code, RAX, dst_reg);
            encode_cqo (code);
            encode_idiv_reg (code, src_reg);
            encode_mov_reg_reg (code, dst_reg, RAX);
        }
    }
    // ========== CALL ========== //
    else if (strcmp (token, "call") == 0)
    {
        char* func_name = strtok (NULL, " ,");
        if (!func_name) return;

        // special syscall wrappers
        if (strcmp (func_name, "scanf") == 0)
        {
            // call in_syscall (implemented in our runtime)
            struct Label* label = find_label (state, "in_syscall");
            if (label && label->is_resolved)
            {
                int32_t rel_offset = (int32_t)(label->code_offset - (get_text_offset (state->elf) + 5));
                encode_call_rel32 (code, rel_offset);
            }
            else
            {
                fprintf (stderr, "Error: in_syscall label not found or not resolved\n");
            }
        }
        else if (strcmp (func_name, "printf") == 0)
        {
            // call out_syscall
            struct Label* label = find_label (state, "out_syscall");
            if (label && label->is_resolved)
            {
                int32_t rel_offset = (int32_t)(label->code_offset - (get_text_offset (state->elf) + 5));
                encode_call_rel32 (code, rel_offset);
            }
            else
            {
                fprintf (stderr, "Error: out_syscall label not found or not resolved\n");
            }
        }
        else
        {
            // regular function call
            struct Label* label = add_label (state, func_name);
            int32_t rel_offset = label->is_resolved ?
                (int32_t)(label->code_offset - (get_text_offset (state->elf) + 5)) : 0;
            encode_call_rel32 (code, rel_offset);
        }
    }
    // ========== WHILE ========== //
    else if (strcmp (token, "while") == 0)
    {
        char* condition = strtok (NULL, ",");
        char* label_name = strtok (NULL, " ,");

        if (!condition || !label_name) return;

        // push loop label to stack
        if (state->loop_stack_depth < MAX_LABELS)
        {
            strncpy (state->loop_stack[state->loop_stack_depth], label_name, MAX_LENGTH_NAME - 1);
            state->loop_stack[state->loop_stack_depth][MAX_LENGTH_NAME - 1] = '\0';
            state->loop_stack_depth++;
        }

        // resolve loop label
        resolve_label (state, label_name, get_text_offset (state->elf));

        // parse condition: "r2 > 0"
        char cond_copy[128] = {};
        strncpy (cond_copy, condition, sizeof (cond_copy) - 1);

        char* reg_str = strtok (cond_copy, " ");
        char* op = strtok (NULL, " ");
        char* num_str = strtok (NULL, " ");

        if (reg_str && op && num_str && strcmp (op, ">") == 0)
        {
            Register reg = parse_register (reg_str);
            int32_t value = atoi (num_str);

            encode_cmp_reg_imm (code, reg, value);

            // create end_loop label
            char end_label[64] = {};
            snprintf (end_label, sizeof (end_label), "end_loop_%s", label_name);
            add_label (state, end_label);

            // emit jle with placeholder offset (will be patched in end_while)
            size_t jle_offset = get_text_offset (state->elf);
            encode_jle_rel32 (code, 0);  // placeholder

            // remember this jump needs patching
            if (state->patch_count < MAX_LABELS)
            {
                state->patches[state->patch_count].patch_offset = jle_offset + 2;  // +2 to skip opcode bytes (0f 8e)
                strncpy (state->patches[state->patch_count].target_label, end_label, MAX_LENGTH_NAME - 1);
                state->patches[state->patch_count].target_label[MAX_LENGTH_NAME - 1] = '\0';
                fprintf (stderr, "  Saving patch: jle at 0x%lx -> %s\n", jle_offset, end_label);
                state->patch_count++;
            }
        }
    }
    // ========== END_WHILE ========== //
    else if (strcmp (token, "end_while") == 0)
    {
        // pop loop label from stack
        if (state->loop_stack_depth > 0)
        {
            state->loop_stack_depth--;
            char* loop_label = state->loop_stack[state->loop_stack_depth];

            // find loop start label
            struct Label* start_label = find_label (state, loop_label);
            if (start_label && start_label->is_resolved)
            {
                // jmp back to loop start
                int32_t rel_offset = (int32_t)(start_label->code_offset - (get_text_offset (state->elf) + 5));
                encode_jmp_rel32 (code, rel_offset);
            }

            // resolve end_loop label at current position
            char end_label[64] = {};
            snprintf (end_label, sizeof (end_label), "end_loop_%s", loop_label);
            size_t end_offset = get_text_offset (state->elf);
            resolve_label (state, end_label, end_offset);

            // patch all pending jumps to this end_loop label
            fprintf (stderr, "  Patching jumps to %s at offset 0x%lx (buffer size=%lu)\n",
                     end_label, end_offset, code->size);
            for (int i = 0; i < state->patch_count; i++)
            {
                if (strcmp (state->patches[i].target_label, end_label) == 0)
                {
                    // calculate relative offset: target - (patch_location + 4)
                    // patch_location points to the rel32 field (after opcode)
                    size_t patch_loc = state->patches[i].patch_offset;
                    int32_t rel_offset = (int32_t)(end_offset - (patch_loc + 4));
                    fprintf (stderr, "    Patching buffer[0x%lx]: rel_offset=%d (0x%x), old_bytes=%02x %02x %02x %02x\n",
                             patch_loc, rel_offset, (uint32_t)rel_offset,
                             code->data[patch_loc], code->data[patch_loc+1],
                             code->data[patch_loc+2], code->data[patch_loc+3]);
                    patch_rel32 (code, patch_loc, rel_offset);
                    fprintf (stderr, "    After patch: %02x %02x %02x %02x\n",
                             code->data[patch_loc], code->data[patch_loc+1],
                             code->data[patch_loc+2], code->data[patch_loc+3]);

                    // remove this patch by moving last element here
                    state->patches[i] = state->patches[state->patch_count - 1];
                    state->patch_count--;
                    i--;  // recheck this index
                }
            }
        }
    }
    // ========== SQRT ========== //
    // IR format: "sqrt rX"
    // codegen:   cvtsi2sd xmm0, rX  ->  sqrtsd xmm0, xmm0  ->  cvttsd2si rX, xmm0
    else if (strcmp (token, "sqrt") == 0)
    {
        char* reg_str = strtok (NULL, " ,");
        if (!reg_str) return;

        if (is_register (reg_str))
        {
            Register reg = parse_register (reg_str);
            encode_cvtsi2sd_xmm0_reg  (code, reg);   // int -> double
            encode_sqrtsd_xmm0_xmm0   (code);        // sqrt
            encode_cvttsd2si_reg_xmm0 (code, reg);   // double -> int (truncate)
        }
    }
    // ========== IF ========== //
    // IR format: "if rX != 0, label"
    else if (strcmp (token, "if") == 0)
    {
        char* condition  = strtok (NULL, ",");
        char* label_name = strtok (NULL, " ,");

        if (!condition || !label_name) return;

        // push if label to stack
        if (state->if_stack_depth < MAX_LABELS)
        {
            strncpy (state->if_stack[state->if_stack_depth], label_name, MAX_LENGTH_NAME - 1);
            state->if_stack[state->if_stack_depth][MAX_LENGTH_NAME - 1] = '\0';
            state->if_stack_depth++;
        }

        // parse condition: "rX != 0"
        char cond_copy[128] = {};
        strncpy (cond_copy, condition, sizeof (cond_copy) - 1);

        char* reg_str = strtok (cond_copy, " ");
        // skip op ("!=") and value ("0") — we only support != 0 rn
        Register reg = parse_register (reg_str);

        // test rX, rX  (sets ZF if rX == 0)
        encode_test_reg_reg (code, reg, reg);

        // je end_if_label  (jump over body if zero)
        char end_label[64] = {};
        snprintf (end_label, sizeof (end_label), "end_if_%s", label_name);
        add_label (state, end_label);

        size_t je_offset = get_text_offset (state->elf);
        encode_je_rel32 (code, 0);  // placeholder

        if (state->patch_count < MAX_LABELS)
        {
            state->patches[state->patch_count].patch_offset = je_offset + 2;  // skip 0F 84
            strncpy (state->patches[state->patch_count].target_label, end_label, MAX_LENGTH_NAME - 1);
            state->patches[state->patch_count].target_label[MAX_LENGTH_NAME - 1] = '\0';
            state->patch_count++;
        }
    }
    // ========== END_IF ========== //
    else if (strcmp (token, "end_if") == 0)
    {
        if (state->if_stack_depth > 0)
        {
            state->if_stack_depth--;
            char* if_label = state->if_stack[state->if_stack_depth];

            // resolve end_if label at current position
            char end_label[64] = {};
            snprintf (end_label, sizeof (end_label), "end_if_%s", if_label);
            size_t end_offset = get_text_offset (state->elf);
            resolve_label (state, end_label, end_offset);

            // patch all pending je jumps to this end_if label
            for (int i = 0; i < state->patch_count; i++)
            {
                if (strcmp (state->patches[i].target_label, end_label) == 0)
                {
                    size_t patch_loc = state->patches[i].patch_offset;
                    int32_t rel_offset = (int32_t)(end_offset - (patch_loc + 4));
                    patch_rel32 (code, patch_loc, rel_offset);

                    state->patches[i] = state->patches[state->patch_count - 1];
                    state->patch_count--;
                    i--;
                }
            }
        }
    }
    // ========== PARAM ========== //
    else if (strcmp (token, "param") == 0)
    {
        // skipped because rn funcs never uses
        // its parameter - input is always read via in_syscall instead.
    }
    else
    {
        fprintf (stderr, "Warning: unknown IR instruction: %s\n", token);
    }
}

// ========== runtime function emission ========== //

// in_syscall: reads integer from stdin, returns result in RAX
// ported from io_syscalls.nasm
static void emit_in_syscall (struct CompilerState* state, uint64_t buffer_addr)
{
    struct CodeBuffer* code = get_text_buffer (state->elf);
    resolve_label (state, "in_syscall", get_text_offset (state->elf));

    // save caller-saved registers
    encode_push_reg (code, RBX);
    encode_push_reg (code, RCX);
    encode_push_reg (code, RDX);
    encode_push_reg (code, RSI);
    encode_push_reg (code, RDI);

    // sys_read(0, buffer, 16)
    encode_mov_reg_imm64 (code, RAX, 0);                // rax = 0 (sys_read)
    encode_mov_reg_imm64 (code, RDI, 0);                // rdi = 0 (stdin)
    encode_mov_reg_imm64 (code, RSI, buffer_addr);      // rsi = buffer
    encode_mov_reg_imm64 (code, RDX, 16);               // rdx = 16 bytes
    encode_syscall (code);

    // prepare for ASCII->number conversion
    encode_mov_reg_imm64 (code, RSI, buffer_addr);      // rsi = buffer
    encode_xor_reg_reg (code, RAX, RAX);                // rax = 0 (result)
    encode_xor_reg_reg (code, RBX, RBX);                // rbx = 0 (temp)

    // .convert_loop:
    size_t convert_loop = get_text_offset (state->elf);

    encode_mov_bl_mem (code, RSI, 0);                   // mov bl, [rsi]

    // cmp bl, 0 -> je .done
    encode_cmp_bl_imm8 (code, 0);
    size_t je_done_1 = get_text_offset (state->elf);
    encode_je_rel32 (code, 0);                          // placeholder

    // cmp bl, 10 -> je .done
    encode_cmp_bl_imm8 (code, 10);
    size_t je_done_2 = get_text_offset (state->elf);
    encode_je_rel32 (code, 0);                          // placeholder

    // sub bl, '0'
    encode_sub_bl_imm8 (code, '0');

    // cmp bl, 9 -> ja .error
    encode_cmp_bl_imm8 (code, 9);
    size_t ja_error = get_text_offset (state->elf);
    encode_ja_rel32 (code, 0);                          // placeholder

    // imul rax, 10
    encode_imul_reg_imm (code, RAX, 10);

    // add rax, rbx
    encode_add_reg_reg (code, RAX, RBX);

    // inc rsi
    encode_inc_reg (code, RSI);

    // jmp .convert_loop (backward jump)
    int32_t jmp_back = (int32_t)(convert_loop - (get_text_offset (state->elf) + 5));
    encode_jmp_rel32 (code, jmp_back);

    // .done:
    size_t done = get_text_offset (state->elf);

    // patch forward jumps to .done
    patch_rel32 (code, je_done_1 + 2, (int32_t)(done - (je_done_1 + 6)));
    patch_rel32 (code, je_done_2 + 2, (int32_t)(done - (je_done_2 + 6)));

    // restore registers and return
    encode_pop_reg (code, RDI);
    encode_pop_reg (code, RSI);
    encode_pop_reg (code, RDX);
    encode_pop_reg (code, RCX);
    encode_pop_reg (code, RBX);
    encode_ret (code);

    // .error:
    size_t error = get_text_offset (state->elf);

    // patch ja .error
    patch_rel32 (code, ja_error + 2, (int32_t)(error - (ja_error + 6)));

    encode_mov_reg_imm64 (code, RAX, (uint64_t)-1);    // rax = -1
    encode_ret (code);
}

// out_syscall: prints integer from RDI to stdout
// ported from io_syscalls.nasm
static void emit_out_syscall (struct CompilerState* state, uint64_t output_buffer_addr)
{
    struct CodeBuffer* code = get_text_buffer (state->elf);
    resolve_label (state, "out_syscall", get_text_offset (state->elf));

    encode_mov_reg_reg (code, RAX, RDI);                    // rax = input value
    encode_mov_reg_imm64 (code, RDI, output_buffer_addr + 15);  // rdi = end of buffer
    encode_mov_byte_reg_ind_imm8 (code, RDI, 10);           // mov byte [rdi], '\n'
    encode_dec_reg (code, RDI);                             // dec rdi
    encode_mov_reg_imm64 (code, RCX, 0);                    // rcx = 0 (digit counter)

    // test rax, rax -> jnz .convert
    encode_test_reg_reg (code, RAX, RAX);
    size_t jnz_convert = get_text_offset (state->elf);
    encode_jnz_rel32 (code, 0);                             // placeholder

    // zero case: write '0'
    encode_mov_byte_reg_ind_imm8 (code, RDI, '0');          // mov byte [rdi], '0'
    encode_inc_reg (code, RCX);                             // inc rcx
    size_t jmp_write = get_text_offset (state->elf);
    encode_jmp_rel32 (code, 0);                             // placeholder -> .write

    // .convert:
    size_t convert = get_text_offset (state->elf);

    // patch jnz .convert
    patch_rel32 (code, jnz_convert + 2, (int32_t)(convert - (jnz_convert + 6)));

    encode_mov_reg_imm64 (code, RBX, 10);                   // rbx = 10 (divisor)

    // .convert_loop:
    size_t convert_loop = get_text_offset (state->elf);

    encode_xor_reg_reg (code, RDX, RDX);                    // xor rdx, rdx
    encode_div_reg (code, RBX);                             // div rbx (unsigned)
    encode_add_dl_imm8 (code, '0');                         // add dl, '0'
    encode_mov_reg_ind_dl (code, RDI);                      // mov [rdi], dl
    encode_dec_reg (code, RDI);                             // dec rdi
    encode_inc_reg (code, RCX);                             // inc rcx

    // test rax, rax -> jnz .convert_loop (backward jump)
    encode_test_reg_reg (code, RAX, RAX);
    int32_t jnz_back = (int32_t)(convert_loop - (get_text_offset (state->elf) + 6));
    encode_jnz_rel32 (code, jnz_back);

    // .write:
    size_t write = get_text_offset (state->elf);

    // patch jmp .write
    patch_rel32 (code, jmp_write + 1, (int32_t)(write - (jmp_write + 5)));

    encode_inc_reg (code, RDI);                             // inc rdi (point to first digit)
    encode_mov_reg_imm64 (code, RAX, 1);                    // rax = 1 (sys_write)
    encode_mov_reg_reg (code, RSI, RDI);                    // rsi = buffer address
    encode_mov_reg_imm64 (code, RDI, 1);                    // rdi = 1 (stdout)
    encode_mov_reg_reg (code, RDX, RCX);                    // rdx = digit count
    encode_inc_reg (code, RDX);                             // rdx++ (for newline)
    encode_syscall (code);
    encode_ret (code);
}

// hlt_syscall: terminates the program with exit code 0
static void emit_hlt_syscall (struct CompilerState* state)
{
    struct CodeBuffer* code = get_text_buffer (state->elf);
    resolve_label (state, "hlt_syscall", get_text_offset (state->elf));

    encode_mov_reg_imm64 (code, RAX, 60);   // rax = 60 (sys_exit)
    encode_xor_reg_reg (code, RDI, RDI);    // rdi = 0 (exit code)
    encode_syscall (code);
    encode_ret (code);
}

static size_t emit_runtime_functions (struct CompilerState* state)
{
    size_t runtime_start = get_text_offset (state->elf);

    // allocate input buffer in .data section (256 bytes)
    uint64_t buffer_addr = get_data_absolute_addr (state->elf, get_data_offset (state->elf));
    for (int i = 0; i < 32; i++)
        add_data_qword (state->elf, 0);

    // allocate output buffer in .data section (16 bytes)
    uint64_t output_buffer_addr = get_data_absolute_addr (state->elf, get_data_offset (state->elf));
    for (int i = 0; i < 2; i++)
        add_data_qword (state->elf, 0);

    emit_in_syscall  (state, buffer_addr);
    emit_out_syscall (state, output_buffer_addr);
    emit_hlt_syscall (state);

    return runtime_start;
}

// ========== main compilation function ========== //

enum Errors generate_elf_binary (struct IRGenerator_t* gen, const char* output_filename)
{
    assert (gen);
    assert (output_filename);

    struct CompilerState state = {};
    state.elf = create_elf_builder();
    state.label_count = 0;
    state.variable_count = 0;
    state.loop_stack_depth = 0;
    state.patch_count = 0;

    // emit runtime functions first
    emit_runtime_functions (&state);

    // compile user code
    for (int i = 0; i < gen->instr_count; i++)
        compile_ir_instruction (&state, gen->instructions[i]);

    // create _start function after user code
    struct CodeBuffer* code = get_text_buffer (state.elf);
    size_t start_offset = get_text_offset (state.elf);  // save offset BEFORE adding any code

    fprintf (stderr, "Creating _start at offset 0x%lx (text_size=%lu)\n", start_offset, code->size);

    // resolve _start label at current position
    resolve_label (&state, "_start", start_offset);

    // initialize stack: kernel should set rsp, but clear rbp for stack unwinding
    encode_xor_reg_reg (code, RBP, RBP);  // xor rbp, rbp (mark end of stack frames)

    // emit _start code: call carti
    struct Label* carti_label = find_label (&state, "carti");
    if (carti_label && carti_label->is_resolved)
    {
        size_t call_offset = start_offset + 3;  // after xor rbp,rbp (3 bytes)
        int32_t rel_offset = (int32_t)(carti_label->code_offset - (call_offset + 5));
        encode_call_rel32 (code, rel_offset);
        fprintf (stderr, "  call carti: offset=0x%lx, rel_offset=%d\n", call_offset, rel_offset);
    }
    else
    {
        fprintf (stderr, "Error: carti label not found\n");
    }

    // emit _start code: call hlt_syscall
    struct Label* hlt_label = find_label (&state, "hlt_syscall");
    if (hlt_label && hlt_label->is_resolved)
    {
        size_t call_offset = start_offset + 3 + 5;  // after xor (3) + first call (5)
        int32_t rel_offset = (int32_t)(hlt_label->code_offset - (call_offset + 5));
        encode_call_rel32 (code, rel_offset);
        fprintf (stderr, "  call hlt_syscall: offset=0x%lx, rel_offset=%d\n", call_offset, rel_offset);
    }
    else
    {
        fprintf (stderr, "Error: hlt_syscall label not found\n");
    }

    // set entry point to _start (using saved offset)
    // entry point virtual address = TEXT_VADDR + file_offset
    // file_offset = ELF headers size + offset of _start inside .text buffer
    size_t file_offset = get_header_size() + start_offset;

    fprintf (stderr, "Setting entry point: text_offset=0x%lx, file_offset=0x%lx, vaddr=0x%lx\n",
             start_offset, file_offset, TEXT_VADDR + file_offset);
    set_entry_point (state.elf, file_offset);

    // write executable
    int result = write_elf_executable (state.elf, output_filename);

    destroy_elf_builder (state.elf);

    return (result == 0) ? NO_ERROR : FILE_OPEN_ERROR;
}

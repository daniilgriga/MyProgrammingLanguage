#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "backend.h"

#define MAX_LINE        256
#define MAX_TOKENS      100
#define MAX_REGISTERS   14
#define MAX_VARIABLES   50
#define MAX_LENGTH_NAME 32
#define MAX_STACK_DEPTH 10

struct Token
{
    char value[MAX_LENGTH_NAME];
    int is_register;                    // 1 - register, 0 - no register
};

struct IR_Transformation
{
    const char* ir_op;                  // op name     (example: "set")
    const char* x86_op;                 // op_x86_name (example: "mov")
    int arg_count;                      // count of arguments
    int is_variable_target;             // 1, if first  arg - var
    int is_number_allowed;              // 1, if second arg may be num
};

struct IR_Transformation transformations[] = {
    {"set",          "mov",     2, 0, 1}, // < set rX, rY or set rX, NUM >
    {"store",        "mov",     2, 1, 0}, // < store VAR, rX >
    {"push",         "push",    1, 0, 0}, // < push rX   >
    {"call",         "call",    1, 0, 0}, // < call FUNC >
    {"pop",          "pop",     1, 0, 1}, // < pop N     > (clean stack, equals add <rsp, N>)
    {"function",     "label",   1, 0, 0}, // < function FUNC >
    {"end_function", "ret",     0, 0, 0}, // < end_function >
    {"while",        "loop",    2, 0, 0}, // < money (condition, body_label) >
    {"end_while",    "endloop", 0, 0, 0}, // < endloop > (end of cycle)
    {"add",          "add",     2, 0, 0}, // < add rX, rY >
    {"sub",          "sub",     2, 0, 0}, // < sub rX, rY >
    {"mul",          "imul",    2, 0, 0}, // < mul rX, rY >
    {"div",          "idiv",    2, 0, 0}, // < div rX, rY >
    {NULL,           NULL,      0, 0, 0}  //  end of table
};

struct Variable
{
    char name[MAX_LENGTH_NAME];
};

const char* reg_map[MAX_REGISTERS] = {
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8",
    "r9" , "r10", "r11", "r12", "r13", "r14", "r15"
};

struct Variable variables[MAX_VARIABLES];
int variable_count = 0;

char stack_labels[MAX_STACK_DEPTH][MAX_LENGTH_NAME]; // stack for labels
int stack_depth = 0;

//=#############################################################=//

static int is_number (const char* str)
{
    assert (str);

    char* endptr;
    strtol (str, &endptr, 10);
    return (*endptr == '\0');
}

static int is_register (const char* str)
{
    return (strlen(str) > 1 && str[0] == 'r' && is_number(str + 1));
}

static void add_variable (const char* name)
{
    assert (name);

    for (int i = 0; i < variable_count; i++)
        if (strcmp(variables[i].name, name) == 0) return;

    if (variable_count < MAX_VARIABLES)
    {
        strncpy (variables[variable_count].name, name, MAX_LENGTH_NAME - 1);
        variable_count++;
    }
    else
    {
        fprintf (stderr, "Error: Too many variables\n");
        exit(1);
    }
}

static int tokenize_line (const char* line, struct Token* tokens)
{
    assert (line);
    assert (tokens);

    char* line_copy = strdup (line);
    char* token = strtok (line_copy, " ,");
    int token_count = 0;

    while (token && token_count < MAX_TOKENS)
    {
        if (strcmp(token, "while") == 0)                                        // for 'loop'
        {
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
            tokens[token_count].is_register = 0;
            token_count++;

            token = strtok (NULL, ",");                                         // condition
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            if (token)
            {
                while (*token == ' ') token++;                                  // skip spaces

                strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
                tokens[token_count].is_register = 0;
                token_count++;
            }                                                                   // ? errors hangling ?

            token = strtok (NULL, " ");                                         // label
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            if (token)
            {
                while (*token == ' ') token++;

                strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
                tokens[token_count].is_register = 0;
                token_count++;
            }

            token = strtok (NULL, " ,");
        }
        else                                                                    // std tokenization:
        {
            strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
            tokens[token_count].is_register = is_register (token);

            fprintf (stderr, "token = %s\n", token);

            token = strtok (NULL, " ,");
            token_count++;
        }
    }

    free (line_copy);

    return token_count;
}

static int get_reg_index (const char* reg_str)
{
    assert (reg_str);

    if (strlen(reg_str) < 2 || reg_str[0] != 'r') return -1;

    int index = atoi (reg_str + 1) - 1;

    return (index >= 0 && index < MAX_REGISTERS) ? index : -1;
}

static void transform_to_x86 (FILE* asm_file, struct Token* tokens, int token_count)
{
    assert (asm_file);
    assert (tokens);

    if (token_count == 0) return;

    for (int i = 0; transformations[i].ir_op; i++)
    {
        if (strcmp(tokens[0].value, transformations[i].ir_op) == 0)
        {
            if (token_count - 1 != transformations[i].arg_count)
            {
                fprintf (stderr, "Error: Wrong number of arguments for %s\n", tokens[0].value);
                return;
            }

            if (strcmp(transformations[i].x86_op, "label") == 0)                                            // | LABEL | //
            {
                fprintf (asm_file, "\n%s:\n", tokens[1].value);
                fprintf (asm_file, "    push rbp\n");
                fprintf (asm_file, "    mov rbp, rsp\n");
            }
            else if (strcmp(transformations[i].x86_op, "ret") == 0)                                         // | RET | //
            {
                fprintf (asm_file, "    mov rsp, rbp\n");
                fprintf (asm_file, "    pop rbp\n");
                fprintf (asm_file, "    ret\n");
            }
            else if (strcmp(transformations[i].x86_op, "mov") == 0)                                         // | MOVE | //
            {
                int reg1_index = tokens[1].is_register ? get_reg_index (tokens[1].value) : -1;
                int reg2_index = tokens[2].is_register ? get_reg_index (tokens[2].value) : -1;

                if (transformations[i].is_variable_target && !tokens[1].is_register)
                {
                    // store VAR, rX
                    add_variable (tokens[1].value);
                    if (reg2_index >= 0 && reg2_index < MAX_REGISTERS)
                        fprintf (asm_file, "    mov [%s], %s\n", tokens[1].value, reg_map[reg2_index]);
                }
                else if (reg1_index >= 0 && reg1_index < MAX_REGISTERS)
                {
                    // set rX, rY or set rX, NUM
                    if (reg2_index >= 0 && reg2_index < MAX_REGISTERS)
                        fprintf (asm_file, "    mov %s, %s\n", reg_map[reg1_index], reg_map[reg2_index]);

                    else if (transformations[i].is_number_allowed && is_number(tokens[2].value))
                        fprintf (asm_file, "    mov %s, %s\n", reg_map[reg1_index], tokens[2].value);
                }
            }
            else if (strcmp(transformations[i].x86_op, "push") == 0)                                        // | PUSH | //
            {
                int reg_idx = get_reg_index (tokens[1].value);

                if (reg_idx >= 0 && reg_idx < MAX_REGISTERS)
                    fprintf (asm_file, "    push %s\n", reg_map[reg_idx]);
            }
            else if (strcmp(transformations[i].x86_op, "call") == 0)                                        // | CALL | //
            {
                fprintf (asm_file, "    call %s\n", tokens[1].value);
            }
            else if (strcmp(transformations[i].x86_op, "pop") == 0)                                         // | POP | //
            {
                if (is_number(tokens[1].value))
                    fprintf (asm_file, "    add rsp, %d\n", atoi(tokens[1].value) * 8);     // 8 byte <=> 1 addr
            }
            else if (strcmp(transformations[i].x86_op, "add") == 0 ||                                       // | ADD | SUB | IMUL | //
                     strcmp(transformations[i].x86_op, "sub") == 0 ||
                     strcmp(transformations[i].x86_op, "imul") == 0)
            {
                int reg1_index = get_reg_index (tokens[1].value);
                int reg2_index = get_reg_index (tokens[2].value);

                if (reg1_index >= 0 && reg1_index < MAX_REGISTERS &&
                    reg2_index >= 0 && reg2_index < MAX_REGISTERS)
                {
                    if (strcmp(transformations[i].x86_op, "add") == 0)                                      // | ADD | //
                    {
                        fprintf(asm_file, "    add %s, %s\n", reg_map[reg1_index], reg_map[reg2_index]);
                    }
                    else if (strcmp(transformations[i].x86_op, "sub") == 0)                                 // | SUB | //
                    {
                        fprintf(asm_file, "    sub %s, %s\n", reg_map[reg1_index], reg_map[reg2_index]);
                    }
                    else if (strcmp(transformations[i].x86_op, "imul") == 0)                                // | IMUL | //
                    {
                        fprintf(asm_file, "    imul %s, %s\n", reg_map[reg1_index], reg_map[reg2_index]);
                    }
                }
            }
            else if (strcmp(transformations[i].x86_op, "idiv") == 0)                                        // | IDIV | //
            {
                int reg1_index = get_reg_index(tokens[1].value);            // dividend
                int reg2_index = get_reg_index(tokens[2].value);            // divisor

                if (reg1_index >= 0 && reg1_index < MAX_REGISTERS &&
                    reg2_index >= 0 && reg2_index < MAX_REGISTERS)
                {
                    fprintf (asm_file, "    mov rax, %s\n", reg_map[reg1_index]);
                    fprintf (asm_file, "    idiv %s\n", reg_map[reg2_index]);
                    fprintf (asm_file, "    mov %s, rax\n", reg_map[reg1_index]);
                }
            }
            else if (strcmp(transformations[i].x86_op, "loop") == 0)                                        // | LOOP | //
            {
                char* condition  = tokens[1].value;     // condition
                char* body_label = tokens[2].value;     // body of cycle

                if (stack_depth < MAX_STACK_DEPTH)
                {
                    strncpy (stack_labels[stack_depth], body_label, MAX_LENGTH_NAME - 1);
                    stack_depth++;
                }
                else
                {
                    fprintf (stderr, "Error: Too many nested loops\n");
                    return;
                }

                fprintf (asm_file, "\nloop_%s:\n", body_label);

                char reg_str[3] = {};
                int num = 0;
                sscanf (condition, "r%s > %d", &reg_str[1], &num);
                reg_str[0] = 'r';
                reg_str[2] = '\0';

                int reg_index = get_reg_index (reg_str);
                fprintf (stderr, "\n [%s] reg_index = %d\n\n", reg_str, reg_index);

                if (reg_index >= 0 && reg_index < MAX_REGISTERS)
                {
                    fprintf (asm_file, "    cmp %s, %d\n", reg_map[reg_index], num);
                    fprintf (asm_file, "    jle end_loop_%s\n", body_label);
                }
            }
            else if (strcmp(transformations[i].x86_op, "endloop") == 0)                                     // | ENDLOOP | //
            {
                if (stack_depth > 0)
                {
                    stack_depth--;

                    char* body_label = stack_labels[stack_depth];

                    fprintf (asm_file, "    jmp loop_%s\n", body_label);
                    fprintf (asm_file, "\nend_loop_%s:\n", body_label);
                }
                else
                {
                    fprintf (stderr, "Error: end_money without matching money\n");
                    return;
                }
            }

            break;
        }
    }
}

void generate_x86_backend (const char* ir_filename, const char* asm_filename)
{
    assert (ir_filename);
    assert (asm_filename);

    FILE* ir_file  = fopen (ir_filename, "rb");
    FILE* asm_file = fopen (asm_filename, "wb");

    if (ir_file == NULL || asm_file == NULL)
    {
        fprintf (stderr, "Error: Cannot open files\n");
        exit(1);
    }

    char line[MAX_LINE] = {};
    struct Token tokens[MAX_TOKENS] = {};

    fprintf (asm_file, "section .data\n");
    while (fgets(line, MAX_LINE, ir_file))
    {
        line[strcspn(line, "\n")] = 0;
        int token_count = tokenize_line (line, tokens);
        if (token_count > 1 && strcmp(tokens[0].value, "store") == 0 && !tokens[1].is_register)
            add_variable (tokens[1].value);
    }

    rewind (ir_file);

    for (int i = 0; i < variable_count; i++)
        fprintf (asm_file, "    %-*s dq 0\n", 10, variables[i].name);

    fprintf (asm_file, "\nsection .text\n");
    fprintf (asm_file, "global _start\n");

    while (fgets(line, MAX_LINE, ir_file))
    {
        line[strcspn(line, "\n")] = 0;
        int token_count = tokenize_line (line, tokens);
        if (token_count > 0)
            transform_to_x86 (asm_file, tokens, token_count);
    }

    fprintf (asm_file, "\n_start:\n");
    fprintf (asm_file, "    call carti\n");
    fprintf (asm_file, "    mov rax, 60\n");
    fprintf (asm_file, "    xor rdi, rdi\n");
    fprintf (asm_file, "    syscall\n");

    fclose (ir_file);
    fclose (asm_file);
}

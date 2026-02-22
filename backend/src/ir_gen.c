#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ir_gen.h"

void initial_ir_generator (struct IRGenerator_t* gen)
{
    gen->symbol_count = 0;
    gen->instr_count = 0;
    gen->reg_count = 0;
}

// allocate a fresh virtual register name into buffer
void new_register (struct IRGenerator_t* gen, char* buffer, size_t size)
{
    assert (gen);
    assert (buffer);

    snprintf (buffer, size, "r%d", ++gen->reg_count);
}

// look up variable by name; return its register
// if not found - allocate a new register, add to table, return it
char* get_or_add_symbol (struct IRGenerator_t* gen, const char* name, int length)
{
    assert (gen);
    assert (name);

    for (int i = 0; i < gen->symbol_count; i++)
    {
        if (strncmp (gen->symbols[i].name, name, (size_t) length) == 0 &&
            gen->symbols[i].name[length] == '\0')
        {
            return strdup (gen->symbols[i].reg);
        }
    }

    if (gen->symbol_count >= MAX_SYMBOLS)
    {
        fprintf (stderr, "Symbol table overflow\n");
        exit (1);
    }

    char reg[MAX_VAR_NAME] = {};
    new_register (gen, reg, sizeof (reg));

    strncpy (gen->symbols[gen->symbol_count].name, name,   MAX_VAR_NAME - 1);
    strncpy (gen->symbols[gen->symbol_count].reg,  reg,    MAX_VAR_NAME - 1);
    gen->symbol_count++;

    return strdup (reg);
}

// update (or insert) the register bound to a variable name
void set_symbol_reg (struct IRGenerator_t* gen, const char* name, int length, const char* reg)
{
    assert (gen);
    assert (name);
    assert (reg);

    for (int i = 0; i < gen->symbol_count; i++)
    {
        if (strncmp (gen->symbols[i].name, name, (size_t) length) == 0 &&
            gen->symbols[i].name[length] == '\0')
        {
            strncpy (gen->symbols[i].reg, reg, MAX_VAR_NAME - 1);
            return;
        }
    }

    if (gen->symbol_count >= MAX_SYMBOLS)
    {
        fprintf (stderr, "Symbol table overflow\n");
        exit (1);
    }

    strncpy (gen->symbols[gen->symbol_count].name, name, MAX_VAR_NAME - 1);
    strncpy (gen->symbols[gen->symbol_count].reg,  reg,  MAX_VAR_NAME - 1);
    gen->symbol_count++;
}

void add_instruction (struct IRGenerator_t* gen, const char* instr)
{
    assert (gen);
    assert (instr);

    if (gen->instr_count < MAX_IR_INSTR)
    {
        strncpy (gen->instructions[gen->instr_count++], instr, MAX_INSTR_LEN - 1);
    }
    else
    {
        fprintf (stderr, "Instruction buffer overflow\n");
        exit (1);
    }
}

// model: each variable owns one fixed register for its lifetime.
char* bypass (struct IRGenerator_t* gen, struct Node_t* node, struct Context_t* context)
{
    assert (gen);
    assert (context);

    if (node == NULL) return NULL;

    switch (node->type)
    {
        case FUNC:
        {
            switch ((int) node->value)
            {
                case FN_GLUE:
                {
                    bypass (gen, node->left,  context);
                    bypass (gen, node->right, context);
                    return NULL;
                }

                case DEF:
                {
                    const char* func_name = context->name_table[(int)node->left->left->value].name.str_pointer;
                    int         func_len  = context->name_table[(int)node->left->left->value].name.length;
                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof (instr), "function %.*s", func_len, func_name);
                    add_instruction (gen, instr);

                    // emit param instructions
                    if (node->left->right &&
                        node->left->right->type == FUNC &&
                        (int) node->left->right->value == COMMA)
                    {
                        struct Node_t* param = node->left->right;
                        while (param)
                        {
                            if (param->left && param->left->type == ID)
                            {
                                const char* pname = context->name_table[(int)param->left->value].name.str_pointer;
                                int         plen  = context->name_table[(int)param->left->value].name.length;
                                char* preg = get_or_add_symbol (gen, pname, plen);

                                snprintf (instr, sizeof (instr), "param %.*s, %s", plen, pname, preg);
                                add_instruction (gen, instr);
                                free (preg);
                            }
                            param = param->right;
                        }
                    }

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof (instr), "end_function");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case CALL:
                {
                    const char* func_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int         func_len  = context->name_table[(int)node->left->value].name.length;
                    char instr[MAX_INSTR_LEN] = {};

                    // for printf: load argument into rdi before call
                    if (strcmp (func_name, "printf") == 0 &&
                        node->right &&
                        node->right->type == FUNC &&
                        (int) node->right->value == COMMA)
                    {
                        struct Node_t* arg = node->right->left;
                        if (arg)
                        {
                            char* arg_reg = bypass (gen, arg, context);
                            if (arg_reg)
                            {
                                snprintf (instr, sizeof (instr), "set rdi, %s", arg_reg);
                                add_instruction (gen, instr);
                                free (arg_reg);
                            }
                        }
                    }

                    snprintf (instr, sizeof (instr), "call %.*s", func_len, func_name);
                    add_instruction (gen, instr);

                    // scanf: result comes back in r0; write it into the variable's register
                    if (strcmp (func_name, "scanf") == 0 &&
                        node->right &&
                        node->right->type == FUNC &&
                        (int) node->right->value == COMMA)
                    {
                        struct Node_t* arg = node->right->left;
                        if (arg && arg->type == ID)
                        {
                            const char* vname = context->name_table[(int)arg->value].name.str_pointer;
                            int         vlen  = context->name_table[(int)arg->value].name.length;

                            char* vreg = get_or_add_symbol (gen, vname, vlen);
                            snprintf (instr, sizeof (instr), "set %s, r0", vreg);
                            add_instruction (gen, instr);
                            free (vreg);
                        }
                    }

                    return NULL;
                }

                case COMMA:
                {
                    bypass (gen, node->left,  context);
                    bypass (gen, node->right, context);
                    return NULL;
                }

                default:
                    fprintf (stderr, "Unknown FUNC value: %.0f\n", node->value);
                    return NULL;
            }
        }

        case OP:
        {
            switch ((int) node->value)
            {
                case GLUE:
                {
                    char* r1 = bypass (gen, node->left,  context);
                    free (r1);
                    char* r2 = bypass (gen, node->right, context);
                    free (r2);
                    return NULL;
                }

                // a is <expr>
                // rA = fixed register for a; write expr result into rA
                case EQUAL:
                {
                    const char* lname = context->name_table[(int)node->left->value].name.str_pointer;
                    int         llen  = context->name_table[(int)node->left->value].name.length;

                    // get (or allocate) lhs's own register
                    char* lreg = get_or_add_symbol (gen, lname, llen);

                    // evaluate rhs: for arithmetic, result is in the LEFT operand's
                    // for ID/NUM bypass returns their reg
                    char* rreg = bypass (gen, node->right, context);

                    if (rreg == NULL)
                    {
                        fprintf (stderr, "Error: rhs is NULL for EQUAL\n");
                        free (lreg);
                        return NULL;
                    }

                    // if rhs already landed in lreg (a is a+b) - nothing to copy
                    if (strcmp (lreg, rreg) != 0)
                    {
                        char instr[MAX_INSTR_LEN] = {};
                        snprintf (instr, sizeof (instr), "set %s, %s", lreg, rreg);
                        add_instruction (gen, instr);
                    }

                    free (rreg);
                    free (lreg);
                    return NULL;
                }

                case ADD:
                case SUB:
                case MUL:
                case DIV:
                case POW:
                {
                    const char* lname = context->name_table[(int)node->left->value].name.str_pointer;
                    int         llen  = context->name_table[(int)node->left->value].name.length;
                    char* lreg = get_or_add_symbol (gen, lname, llen);

                    char* rreg = bypass (gen, node->right, context);

                    char instr[MAX_INSTR_LEN] = {};

                    const char* op = NULL;
                    switch ((int) node->value)
                    {
                        case ADD: op = "add"; break;
                        case SUB: op = "sub"; break;
                        case MUL: op = "mul"; break;
                        case DIV: op = "div"; break;
                        case POW: op = "pow"; break;

                        default:  op = "???"; break;
                    }
                    snprintf (instr, sizeof (instr), "%s %s, %s", op, lreg, rreg);
                    add_instruction (gen, instr);

                    free (rreg);
                    return lreg;  // caller sees updated lreg
                }

                case SQRT:
                {
                    const char* vname = context->name_table[(int)node->left->value].name.str_pointer;
                    int         vlen  = context->name_table[(int)node->left->value].name.length;
                    char* vreg = get_or_add_symbol (gen, vname, vlen);

                    char instr[MAX_INSTR_LEN] = {};
                    snprintf (instr, sizeof (instr), "sqrt %s", vreg);
                    add_instruction (gen, instr);

                    return vreg;
                }

                case WHILE:
                {
                    char label[64] = {};
                    snprintf (label, sizeof (label), "body%d", rand () % 1000);

                    char* cond_reg = bypass (gen, node->left, context);
                    if (cond_reg == NULL)
                    {
                        fprintf (stderr, "Error: condition register is NULL for WHILE\n");
                        return NULL;
                    }

                    char instr[MAX_INSTR_LEN] = {};
                    snprintf (instr, sizeof (instr), "while %s > 0, %s", cond_reg, label);
                    add_instruction (gen, instr);
                    free (cond_reg);

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof (instr), "end_while");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case IF:
                {
                    char label[64] = {};
                    snprintf (label, sizeof (label), "if_body%d", rand () % 1000);

                    char* cond_reg = bypass (gen, node->left, context);
                    if (cond_reg == NULL)
                    {
                        fprintf (stderr, "Error: condition register is NULL for IF\n");
                        return NULL;
                    }

                    char instr[MAX_INSTR_LEN] = {};
                    snprintf (instr, sizeof (instr), "if %s != 0, %s", cond_reg, label);
                    add_instruction (gen, instr);
                    free (cond_reg);

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof (instr), "end_if");
                    add_instruction (gen, instr);

                    return NULL;
                }

                default:
                    fprintf (stderr, "Unknown OP value: %.0f\n", node->value);
                    return NULL;
            }
        }

        case ID:
        {
            const char* name = context->name_table[(int)node->value].name.str_pointer;
            int         len  = context->name_table[(int)node->value].name.length;
            return get_or_add_symbol (gen, name, len);
        }

        case NUM:
        {
            char* reg = malloc (MAX_VAR_NAME);
            new_register (gen, reg, MAX_VAR_NAME);

            char instr[MAX_INSTR_LEN] = {};
            snprintf (instr, sizeof (instr), "set %s, %.0f", reg, node->value);
            add_instruction (gen, instr);

            return reg;
        }

        default:
            fprintf (stderr, "Unknown node type: %d\n", node->type);
            exit (1);
    }
}

int dump_ir_to_file (struct IRGenerator_t* gen, const char* filename)
{
    FILE* file = fopen (filename, "wb");
    if (!file)
    {
        fprintf (stderr, "Error: Cannot open file %s for writing\n", filename);
        return 1;
    }

    for (int i = 0; i < gen->instr_count; i++)
        fprintf (file, "%s\n", gen->instructions[i]);

    fclose (file);

    return 0;
}

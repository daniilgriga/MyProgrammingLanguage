#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ir_gen.h"

void initial_ir_generator (struct IRGenerator* gen)
{
    gen->symbol_count = 0;
    gen->instr_count = 0;
    gen->reg_count = 0;
}

void new_register (struct IRGenerator* gen, char* buffer, size_t size)
{
    assert (gen);
    assert (buffer);

    snprintf (buffer, size, "r%d", ++gen->reg_count);
}

char* get_or_add_symbol (struct IRGenerator* gen, const char* name)
{
    assert (gen);
    assert (name);

    for (int i = 0; i < gen->symbol_count; i++)
    {
        if (strcmp (gen->symbols[i].name, name) == 0)
            return gen->symbols[i].reg;
    }

    if (gen->symbol_count >= MAX_SYMBOLS)
    {
        fprintf (stderr, "Symbol table overflow\n");
        exit(1);
    }

    char* reg = calloc (16, sizeof(char));
    new_register (gen, reg, sizeof(reg));
    //fprintf (stderr, "IN get_or_add_symbol: name = <%s>\n", name);
    strncpy (gen->symbols[gen->symbol_count].name, name, MAX_VAR_NAME);
    strncpy (gen->symbols[gen->symbol_count].reg, reg, MAX_VAR_NAME);

    return gen->symbols[gen->symbol_count++].reg;
}

void add_instruction (struct IRGenerator* gen, const char* instr)
{
    assert (gen);
    assert (instr);

    if (gen->instr_count < MAX_IR_INSTR)
    {
        strncpy (gen->instructions[gen->instr_count++], instr, MAX_INSTR_LEN);
    }
    else
    {
        fprintf (stderr, "Instruction buffer overflow\n");
        exit(1);
    }
}

char* bypass (struct IRGenerator* gen, struct Node_t* node, struct Context_t* context)
{
    assert (gen);
    assert (context);

    if (!node) return NULL;

    switch (node->type)
    {
        case FUNC:
        {
            switch ((int)node->value)
            {
                case FN_GLUE:
                {
                    bypass (gen, node->left, context);
                    bypass (gen, node->right, context);
                    return NULL;
                }

                case DEF:
                {
                    const char* func_name = context->name_table[(int)node->left->left->value].name.str_pointer;
                    int length = context->name_table[(int)node->left->left->value].name.length;
                    fprintf (stderr, "length = %d\n", length);
                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof(instr), "function " "%.*s", length, func_name);
                    add_instruction (gen, instr);

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof(instr), "end_function");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case CALL:
                {
                    const char* func_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int length = context->name_table[(int)node->left->value].name.length;
                    char instr[MAX_INSTR_LEN] = {};
                    snprintf (instr, sizeof(instr), "call " "%.*s", length, func_name);
                    add_instruction (gen, instr);

                    bypass (gen, node->right, context);

                    return NULL;
                }

                case COMMA:
                {
                    bypass (gen, node->left, context);
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
            switch ((int)node->value)
            {
                case GLUE:
                    bypass (gen, node->left, context);
                    bypass (gen, node->right, context);
                    return NULL;

                case EQUAL:
                {
                    const char* var_name = context->name_table[(int)node->left->value].name.str_pointer;
                    char* value_reg = bypass (gen, node->right, context);

                    fprintf (stderr, "| node->left = '%.0f' | node = '%0.f' | node->right = %0.f |\n", node->left->value, node->value, node->right->value);
                    char* var_reg = get_or_add_symbol (gen, var_name);

                    char instr[MAX_INSTR_LEN] = {};
                    if (value_reg)
                    {
                        snprintf (instr, sizeof(instr), "set %s, %s", var_reg, value_reg);
                        add_instruction (gen, instr);
                    }
                    else
                    {
                        fprintf (stderr, "Error: value_reg is NULL for EQUAL\n");
                    }

                    return var_reg;
                }

                case ADD:
                case SUB:
                case MUL:
                case DIV:
                case POW:
                {
                    const char* var_name = context->name_table[(int)node->left->value].name.str_pointer;
                    char* value_reg = bypass (gen, node->right, context);

                    char* var_reg = get_or_add_symbol (gen, var_name);

                    char instr[MAX_INSTR_LEN] = {};

                    if (value_reg)
                    {
                        switch ((int)node->value)
                        {
                            case ADD: snprintf (instr, sizeof(instr), "add %s, %s", var_reg, value_reg); break;
                            case SUB: snprintf (instr, sizeof(instr), "sub %s, %s", var_reg, value_reg); break;
                            case MUL: snprintf (instr, sizeof(instr), "mul %s, %s", var_reg, value_reg); break;
                            case DIV: snprintf (instr, sizeof(instr), "div %s, %s", var_reg, value_reg); break;
                            case POW: snprintf (instr, sizeof(instr), "pow %s, %s", var_reg, value_reg); break;

                            default:
                                fprintf (stderr, "Unknown node value in OP node type: %.0f\n", node->value);
                        }

                        add_instruction (gen, instr);
                    }
                    else
                    {
                        fprintf (stderr, "Error: value_reg is NULL for arithmetic operation\n");
                    }
                }

                default:
                    fprintf (stderr, "Unknown node value in OP node type: %.0f\n", node->value);
                    return NULL;
            }
        }

        case ID:
            return get_or_add_symbol (gen, context->name_table[(int)node->value].name.str_pointer);

        case NUM:
        {
            char* reg = calloc (MAX_VAR_NAME, sizeof (char));
            if (reg == NULL)
            {
                fprintf (stderr, "Memory allocation failed\n");
                exit(1);
            }

            new_register (gen, reg, MAX_VAR_NAME);

            char instr[MAX_INSTR_LEN] = {};

            snprintf (instr, sizeof(instr), "set %s, %.0f", reg, node->value);
            add_instruction (gen, instr);

            return reg;
        }

        default:
            fprintf (stderr, "Unknown node type: %d\n", node->type);
            exit(1);
    }
}

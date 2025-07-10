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

void new_register (struct IRGenerator_t* gen, char* buffer, size_t size)
{
    assert (gen);
    assert (buffer);

    snprintf (buffer, size, "r%d", ++gen->reg_count);
    fprintf (stderr, "in new_register: new reg: [r%d]\n", gen->reg_count);
}

char* get_or_add_symbol (struct IRGenerator_t* gen, const char* name, int length)
{
    assert (gen);
    assert (name);

    for (int i = 0; i < gen->symbol_count; i++)
    {
        fprintf (stderr, "COMPARE: {%d}: [%.10s] | [%.10s]\n", i, gen->symbols[i].name, name);
        if (strncmp(gen->symbols[i].name, name, (size_t) length) == 0)
        {
            char* reg_copy = calloc (MAX_VAR_NAME, sizeof(char));
            if (!reg_copy)
            {
                fprintf (stderr, "Memory allocation failed\n");
                exit(1);
            }

            fprintf (stderr, "get_or_add_symbol: existing reg = <%s> for name = <%.10s>\n", reg_copy, name);
            strncpy (reg_copy, gen->symbols[i].reg, MAX_VAR_NAME);
            return reg_copy;
        }
    }

    if (gen->symbol_count >= MAX_SYMBOLS)
    {
        fprintf (stderr, "Symbol table overflow\n");
        exit(1);
    }

    char reg[MAX_VAR_NAME] = {};
    new_register (gen, reg, sizeof(reg));
    fprintf (stderr, "get_or_add_symbol: new reg = <%s> for name = <%.10s>\n", reg, name);
    strncpy (gen->symbols[gen->symbol_count].name, name, MAX_VAR_NAME);
    strncpy (gen->symbols[gen->symbol_count].reg, reg, MAX_VAR_NAME);

    char* reg_copy = calloc (MAX_VAR_NAME, sizeof(char));
    if (!reg_copy)
    {
        fprintf (stderr, "Memory allocation failed\n");
        exit(1);
    }

    strncpy (reg_copy, gen->symbols[gen->symbol_count].reg, MAX_VAR_NAME);
    gen->symbol_count++;

    return reg_copy;
}

void add_symbol_with_reg (struct IRGenerator_t* gen, const char* name, const char* reg)
{
    assert (gen);
    assert (name);
    assert (reg);

    for (int i = 0; i < gen->symbol_count; i++)
    {
        if (strcmp(gen->symbols[i].name, name) == 0)
        {
            strncpy (gen->symbols[i].reg, reg, MAX_VAR_NAME);
            fprintf (stderr, "add_symbol_with_reg: updated reg = <%s> for name = <%.10s>\n", reg, name);
            return;
        }
    }

    if (gen->symbol_count >= MAX_SYMBOLS)
    {
        fprintf (stderr, "Symbol table overflow\n");
        exit(1);
    }

    strncpy (gen->symbols[gen->symbol_count].name, name, MAX_VAR_NAME);
    strncpy (gen->symbols[gen->symbol_count].reg, reg, MAX_VAR_NAME);
    gen->symbol_count++;

    fprintf (stderr, "add_symbol_with_reg: new reg = <%s> for name = <%.10s>\n", reg, name);
}

void add_instruction (struct IRGenerator_t* gen, const char* instr)
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

char* bypass (struct IRGenerator_t* gen, struct Node_t* node, struct Context_t* context)
{
    assert (gen);
    assert (context);

    if (node == NULL) return NULL;

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
                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof(instr), "function " "%.*s", length, func_name);
                    add_instruction (gen, instr);

                    if (node->left->right && node->left->right->type == FUNC && (int)node->left->right->value == COMMA)
                    {
                        struct Node_t* param = node->left->right;
                        while (param)
                        {
                            if (param->left && param->left->type == ID)
                            {
                                const char* param_name = context->name_table[(int)param->left->value].name.str_pointer;
                                int param_length = context->name_table[(int)param->left->value].name.length;
                                char* param_reg = get_or_add_symbol (gen, param_name, param_length);

                                snprintf (instr, sizeof(instr), "param %.*s, %s", param_length, param_name, param_reg);
                                add_instruction (gen, instr);

                                free (param_reg);
                            }
                            param = param->right;
                        }
                    }

                    char* result_reg = bypass (gen, node->right, context);
                    if (result_reg)
                    {
                        snprintf (instr, sizeof(instr), "set r7, %s", result_reg);
                        add_instruction (gen, instr);
                        free (result_reg);
                    }

                    snprintf (instr, sizeof(instr), "end_function");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case CALL:
                {
                    const char* func_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int length = context->name_table[(int)node->left->value].name.length;
                    char instr[MAX_INSTR_LEN] = {};

                    if (node->right && node->right->type == FUNC && (int)node->right->value == COMMA && strcmp(func_name, "scanf") != 0)
                    {
                        struct Node_t* arg = node->right->left;
                        if (arg && arg->type == ID)
                        {
                            const char* arg_name = context->name_table[(int)arg->value].name.str_pointer;
                            int arg_length = context->name_table[(int)arg->value].name.length;
                            char* arg_reg = get_or_add_symbol (gen, arg_name, arg_length);
                            snprintf (instr, sizeof(instr), "set rdi, %s", arg_reg);
                            add_instruction (gen, instr);
                            free (arg_reg);
                        }
                    }

                    snprintf (instr, sizeof(instr), "call %.*s", length, func_name);
                    add_instruction (gen, instr);

                    char* result = NULL;
                    if (strcmp(func_name, "scanf") == 0)
                    {
                        const char* var_name = context->name_table[(int)node->right->left->value].name.str_pointer;
                        int var_length = context->name_table[(int)node->right->left->value].name.length;
                        char* var_reg = get_or_add_symbol (gen, var_name, var_length);

                        snprintf (instr, sizeof(instr), "set %s, r0", var_reg);
                        add_instruction (gen, instr);

                        result = strdup (var_reg);
                        free (var_reg);
                    }
                    else if (strcmp(func_name, "printf") != 0 && strcmp(func_name, "scanf") != 0)
                    {
                        result = calloc(MAX_VAR_NAME, sizeof(char));
                        if (!result)
                        {
                            fprintf(stderr, "Memory allocation failed\n");
                            exit(1);
                        }
                        new_register(gen, result, MAX_VAR_NAME);
                        snprintf(instr, sizeof(instr), "set %s, r7", result);
                        add_instruction(gen, instr);
                    }

                    return result;
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
                    char* should_free_1 = bypass (gen, node->left, context);
                    free (should_free_1);

                    char* should_free_2 = bypass (gen, node->right, context);
                    free (should_free_2);

                    return NULL;

                case EQUAL:
                {
                    const char* var_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int var_length = context->name_table[(int)node->left->value].name.length;
                    char* value_reg = bypass (gen, node->right, context);

                    char instr[MAX_INSTR_LEN] = {};
                    if (value_reg)
                    {
                        if (node->right->type == NUM)
                        {
                            add_symbol_with_reg (gen, var_name, value_reg);

                            snprintf (instr, sizeof(instr), "store %.*s, %s", var_length, var_name, value_reg);
                            add_instruction (gen, instr);

                            return value_reg;
                        }
                        else if (node->right->type == ID &&                                                                          // |
                                strncmp(context->name_table[(int)node->left->value].name.str_pointer,                                // |
                                        context->name_table[(int)node->right->value].name.str_pointer,                               // |
                                        var_length > context->name_table[(int)node->right->value].name.length ?                      // |
                                        (size_t)var_length : (size_t)context->name_table[(int)node->right->value].name.length) == 0) // | (MIDDLEEND)
                        {                                                                                                            // |
                            free (value_reg);                                                                                        // |
                            return get_or_add_symbol (gen, var_name, var_length);                                                    // |
                        }                                                                                                            // |
                        else
                        {
                            add_symbol_with_reg( gen, var_name, value_reg);

                            snprintf (instr, sizeof(instr), "store %.*s, %s", var_length, var_name, value_reg);
                            add_instruction (gen, instr);

                            return value_reg;
                        }
                    }
                    else
                    {
                        fprintf (stderr, "Error: value_reg is NULL for EQUAL\n");
                    }

                    return NULL;
                }

                case ADD:
                case SUB:
                case MUL:
                case DIV:
                case POW:
                {
                    const char* var_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int var_length = context->name_table[(int)node->left->value].name.length;
                    char* value_reg = bypass (gen, node->right, context);

                    char* var_reg = get_or_add_symbol (gen, var_name, var_length);

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
                        free (value_reg);
                    }
                    else
                    {
                        fprintf (stderr, "Error: value_reg is NULL for arithmetic operation\n");
                    }

                    return var_reg;
                }

                case SQRT:
                {
                    fprintf (stderr, "\n\n\n [%g] \n\n\n", node->left->value);
                    const char* var_name = context->name_table[(int)node->left->value].name.str_pointer;
                    int var_length = context->name_table[(int)node->left->value].name.length;

                    char* var_reg = get_or_add_symbol (gen, var_name, var_length);

                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof(instr), "sqrt %s", var_reg);
                    add_instruction (gen, instr);

                    return var_reg;
                }

                case WHILE:
                {
                    char label[MAX_SYMBOLS] = {};
                    snprintf (label, sizeof(label), "body%d", rand() % 1000);

                    char* counter_reg = bypass (gen, node->left, context);
                    if (counter_reg == NULL)
                    {
                        fprintf (stderr, "Error: Counter register is NULL for MONEY\n");
                        return NULL;
                    }

                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof(instr), "while %s > 0, %s", counter_reg, label);
                    add_instruction (gen, instr);
                    free (counter_reg);

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof(instr), "end_while");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case IF:
                {
                    char label[MAX_SYMBOLS] = {};
                    snprintf (label, sizeof(label), "if_body%d", rand() % 1000);

                    char* condition_reg = bypass(gen, node->left, context);
                    if (condition_reg == NULL)
                    {
                        fprintf (stderr, "Error: Condition register is NULL for IF\n");
                        return NULL;
                    }

                    char instr[MAX_INSTR_LEN] = {};

                    snprintf (instr, sizeof(instr), "if %s != 0, %s", condition_reg, label);
                    add_instruction (gen, instr);
                    free (condition_reg);

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof(instr), "end_if");
                    add_instruction (gen, instr);

                    return NULL;
                }

                default:
                    fprintf (stderr, "Unknown node value in OP node type: %.0f\n", node->value);
                    return NULL;
            }
        }

        case ID:
            return get_or_add_symbol (gen, context->name_table[(int)node->value].name.str_pointer, context->name_table[(int)node->value].name.length);

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

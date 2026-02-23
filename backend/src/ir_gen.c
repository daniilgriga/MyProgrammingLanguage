#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ir_gen.h"

// returns 1 if str looks like a virtual register ("r<digits>")
static int is_register_str (const char* str)
{
    if (!str || str[0] != 'r') return 0;
    for (int i = 1; str[i] != '\0'; i++)
        if (!isdigit ((unsigned char) str[i])) return 0;
    return str[1] != '\0';
}

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

                    // for sqrt: evaluate argument, emit sqrt instruction, return result register
                    if (strncmp (func_name, "sqrt", (size_t) func_len) == 0 &&
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
                                snprintf (instr, sizeof (instr), "sqrt %s", arg_reg);
                                add_instruction (gen, instr);
                                return arg_reg;  // result lands in the same register
                            }
                        }
                        return NULL;
                    }

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

                    // fast path: rhs is a literal - emit set directly, no temp register
                    if (node->right != NULL && node->right->type == NUM)
                    {
                        char instr[MAX_INSTR_LEN] = {};
                        snprintf (instr, sizeof (instr), "set %s, %.0f", lreg, node->right->value);
                        add_instruction (gen, instr);
                        free (lreg);
                        return NULL;
                    }

                    // fast path: rhs is a simple binary op - write result directly into lreg,
                    // avoiding an extra temp register
                    struct Node_t* rhs = node->right;
                    int rhs_op = (rhs != NULL) ? (int) rhs->value : 0;
                    if (rhs != NULL && rhs->type == OP &&
                        (rhs_op == ADD || rhs_op == SUB || rhs_op == MUL ||
                         rhs_op == DIV || rhs_op == POW))
                    {
                        // get operand strings (may be reg or literal)
                        char* a = bypass (gen, rhs->left,  context);
                        char* b = bypass (gen, rhs->right, context);

                        const char* op_str = (rhs_op == ADD) ? "add" :
                                             (rhs_op == SUB) ? "sub" :
                                             (rhs_op == MUL) ? "mul" :
                                             (rhs_op == DIV) ? "div" : "pow";

                        char instr[MAX_INSTR_LEN] = {};

                        // if 'b' aliases lreg, it would be clobbered by "set lreg, a" - save it first
                        if (is_register_str (b) && strcmp (b, lreg) == 0)
                        {
                            char saved[MAX_VAR_NAME] = {};
                            new_register (gen, saved, sizeof (saved));
                            snprintf (instr, sizeof (instr), "set %s, %s", saved, b);
                            add_instruction (gen, instr);
                            free (b);
                            b = strdup (saved);
                        }
                        // if 'a' aliases lreg - no copy (just apply op in lreg)
                        if (!(is_register_str (a) && strcmp (a, lreg) == 0))
                        {
                            snprintf (instr, sizeof (instr), "set %s, %s", lreg, a);
                            add_instruction (gen, instr);
                        }
                        snprintf (instr, sizeof (instr), "%s %s, %s", op_str, lreg, b);
                        add_instruction (gen, instr);

                        free (a);
                        free (b);
                        free (lreg);
                        return NULL;
                    }

                    // evaluate rhs: for arithmetic, result is in a temp register
                    char* rreg = bypass (gen, node->right, context);

                    if (rreg == NULL)
                    {
                        fprintf (stderr, "Error: rhs is NULL for EQUAL\n");
                        free (lreg);
                        return NULL;
                    }

                    // copy result into lhs register if needed
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
                    char* lreg = bypass (gen, node->left,  context);
                    char* rreg = bypass (gen, node->right, context);

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

                    char instr[MAX_INSTR_LEN] = {};

                    // if rreg is a literal number - emit op directly without a temp register
                    // backend handles reg-imm variants (add rX, 4 / mul rX, 4 / etc.)
                    if (!is_register_str (rreg))
                    {
                        // lreg may be a named-variable register - copy into a temp to avoid clobbering it
                        char tmp[MAX_VAR_NAME] = {};
                        new_register (gen, tmp, sizeof (tmp));
                        snprintf (instr, sizeof (instr), "set %s, %s", tmp, lreg);
                        add_instruction (gen, instr);
                        snprintf (instr, sizeof (instr), "%s %s, %s", op, tmp, rreg);
                        add_instruction (gen, instr);
                        free (lreg);
                        free (rreg);
                        return strdup (tmp);
                    }

                    // both operands are registers - copy lreg into a temp to avoid clobbering the variable
                    char tmp[MAX_VAR_NAME] = {};
                    new_register (gen, tmp, sizeof (tmp));
                    snprintf (instr, sizeof (instr), "set %s, %s", tmp, lreg);
                    add_instruction (gen, instr);
                    snprintf (instr, sizeof (instr), "%s %s, %s", op, tmp, rreg);
                    add_instruction (gen, instr);

                    free (lreg);
                    free (rreg);
                    return strdup (tmp);
                }

                case WHILE:
                {
                    char label[64] = {};
                    snprintf (label, sizeof (label), "body%d", rand () % 1000);

                    char instr[MAX_INSTR_LEN] = {};

                    struct Node_t* cond = node->left;
                    int cond_op = (cond != NULL) ? (int) cond->value : 0;

                    if (cond != NULL && cond->type == OP &&
                        (cond_op == GT || cond_op == LT || cond_op == GTE ||
                         cond_op == NEQ || cond_op == EQ))
                    {
                        // comparison: "while rL <op> rR, label"
                        char* lreg = bypass (gen, cond->left,  context);
                        char* rreg = bypass (gen, cond->right, context);
                        const char* op_str = (cond_op == GT)  ? "fr"     :
                                             (cond_op == LT)  ? "lowkey" :
                                             (cond_op == GTE) ? "nocap"  :
                                             (cond_op == NEQ) ? "nah"    : "sameAs";
                        snprintf (instr, sizeof (instr), "while %s %s %s, %s", lreg, op_str, rreg, label);
                        add_instruction (gen, instr);
                        free (lreg);
                        free (rreg);
                    }
                    else
                    {
                        char* cond_reg = bypass (gen, cond, context);
                        if (cond_reg == NULL)
                        {
                            fprintf (stderr, "Error: condition register is NULL for WHILE\n");
                            return NULL;
                        }
                        snprintf (instr, sizeof (instr), "while %s > 0, %s", cond_reg, label);
                        add_instruction (gen, instr);
                        free (cond_reg);
                    }

                    bypass (gen, node->right, context);

                    snprintf (instr, sizeof (instr), "end_while");
                    add_instruction (gen, instr);

                    return NULL;
                }

                case IF:
                {
                    char label[64] = {};
                    snprintf (label, sizeof (label), "if_body%d", rand () % 1000);

                    char instr[MAX_INSTR_LEN] = {};

                    struct Node_t* cond = node->left;
                    int cond_op = (cond != NULL) ? (int) cond->value : 0;

                    if (cond != NULL && cond->type == OP &&
                        (cond_op == GT || cond_op == LT || cond_op == GTE ||
                         cond_op == NEQ || cond_op == EQ))
                    {
                        // comparison: "if rL <op> rR, label"
                        char* lreg = bypass (gen, cond->left,  context);
                        char* rreg = bypass (gen, cond->right, context);
                        const char* op_str = (cond_op == GT)  ? "fr"     :
                                             (cond_op == LT)  ? "lowkey" :
                                             (cond_op == GTE) ? "nocap"  :
                                             (cond_op == NEQ) ? "nah"    : "sameAs";
                        snprintf (instr, sizeof (instr), "if %s %s %s, %s", lreg, op_str, rreg, label);
                        add_instruction (gen, instr);
                        free (lreg);
                        free (rreg);
                    }
                    else
                    {
                        char* cond_reg = bypass (gen, cond, context);
                        if (cond_reg == NULL)
                        {
                            fprintf (stderr, "Error: condition register is NULL for IF\n");
                            return NULL;
                        }
                        snprintf (instr, sizeof (instr), "if %s != 0, %s", cond_reg, label);
                        add_instruction (gen, instr);
                        free (cond_reg);
                    }

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
            // return the literal as a string - no register allocated
            // callers (ADD/SUB/MUL/DIV/SET) must check is_number() on the returned string
            char* str = malloc (MAX_VAR_NAME);
            snprintf (str, MAX_VAR_NAME, "%.0f", node->value);
            return str;
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

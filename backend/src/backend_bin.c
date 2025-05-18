#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include "backend_bin.h"
#include "errors.h"
#include "file.h"

#define MAX_LINE 100
#define MAX_TOKENS 10
#define MAX_COMMANDS 1000
#define MAX_LENGTH_NAME 32
#define MAX_STACK_DEPTH 10
#define MAX_LABELS 100
#define MAX_VARIABLES 100

struct Token
{
    char value[MAX_LENGTH_NAME];
    int is_register;
};

typedef struct
{
    char type[MAX_LENGTH_NAME];
    char args[3][MAX_LENGTH_NAME];
    char label[MAX_LENGTH_NAME];
    int arg_count;
} IR_Command;

struct Label
{
    char name[MAX_LENGTH_NAME];
    int command_index;  // in ir_commands
};

struct Symbol
{
    char name[MAX_LENGTH_NAME];
    uint64_t address;   // in .data
    int size;
};

IR_Command ir_commands[MAX_COMMANDS];
int command_count = 0;

static char stack_labels[MAX_STACK_DEPTH][MAX_LENGTH_NAME]; // stack for labels
static int stack_depth = 0;

static struct Label labels[MAX_LABELS];
static int label_count = 0;

static struct Symbol variables[MAX_VARIABLES];
static int variable_count = 0;

static int is_register (const char* token)
{
    return (token[0] == 'r' && isdigit(token[1]));
}

static void add_label (const char* name, int command_index)
{
    if (label_count >= MAX_LABELS)
    {
        fprintf (stderr, "Error: too many labels\n");
        exit(1);
    }

    strncpy (labels[label_count].name, name, MAX_LENGTH_NAME - 1);
    labels[label_count].name[MAX_LENGTH_NAME - 1] = '\0';
    labels[label_count].command_index = command_index;
    label_count++;
}

static void add_variable (const char* name, uint64_t address, int size)
{
    for (int i = 0; i < variable_count; i++)
        if (strcmp(variables[i].name, name) == 0)
            return;  // already added

    if (variable_count >= MAX_VARIABLES)
    {
        fprintf (stderr, "Error: too many variables\n");
        exit(1);
    }

    strncpy(variables[variable_count].name, name, MAX_LENGTH_NAME - 1);
    variables[variable_count].name[MAX_LENGTH_NAME - 1] = '\0';
    variables[variable_count].address = address;
    variables[variable_count].size = size;
    variable_count++;
}

static int tokenize_line (const char* line, struct Token* tokens)
{
    assert (line);
    assert (tokens);

    char* line_copy = strdup (line);
    if (line_copy == NULL)
    {
        fprintf (stderr, "Error: memory allocation failed in tokenize_line\n");
        return 0;
    }

    char* token = strtok (line_copy, " ,");
    int token_count = 0;

    while (token && token_count < MAX_TOKENS)
    {
        if (strcmp(token, "while") == 0)                                        // for 'loop'
        {
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
            tokens[token_count].value[MAX_LENGTH_NAME - 1] = '\0';
            tokens[token_count].is_register = 0;
            token_count++;

            token = strtok (NULL, ",");                                         // condition
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            if (token)
            {
                while (*token == ' ') token++;                                  // skip spaces

                if (*token == '\0')
                {
                    fprintf (stderr, "Error: empty condition in money\n");
                    free (line_copy);
                    return token_count;
                }

                strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
                tokens[token_count].value[MAX_LENGTH_NAME - 1] = '\0';
                tokens[token_count].is_register = 0;
                token_count++;
            }
            else
            {
                fprintf (stderr, "Error: missing condition in money\n");
                free (line_copy);
                return token_count;
            }


            token = strtok (NULL, " ");                                         // label
            fprintf (stderr, "%d: [%s]\n",token_count, token);
            if (token)
            {
                while (*token == ' ') token++;

                if (*token == '\0')
                {
                    fprintf (stderr, "Error: missing label in money\n");
                    free (line_copy);
                    return token_count;
                }

                strncpy (tokens[token_count].value, token, MAX_LENGTH_NAME - 1);
                tokens[token_count].value[MAX_LENGTH_NAME - 1] = '\0';
                tokens[token_count].is_register = 0;
                token_count++;
            }
            else
            {
                fprintf (stderr, "Error: missing label in money\n");
                free (line_copy);
                return token_count;
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

int parse_IR_to_array (const char* filename)
{
    FILE* file = OpenFile (filename, "r");
    if (file == NULL)
        return 1;

    char line[MAX_LINE];
    struct Token tokens[MAX_TOKENS];
    command_count = 0;


    while (fgets(line, MAX_LINE, file))
    {
        line[strcspn(line, "\n")] = 0;

        int token_count = tokenize_line (line, tokens);
        if (token_count > 0 && command_count < MAX_COMMANDS)
        {
            IR_Command* cmd = &ir_commands[command_count];

            strncpy (cmd->type, tokens[0].value, MAX_LENGTH_NAME - 1);
            cmd->type[MAX_LENGTH_NAME - 1] = '\0';

            cmd->arg_count = token_count - 1;
            for (int i = 0; i < cmd->arg_count && i < 3; i++)
            {
                strncpy (cmd->args[i], tokens[i + 1].value, MAX_LENGTH_NAME - 1);
                cmd->args[i][MAX_LENGTH_NAME - 1] = '\0';
            }

            if (strcmp(cmd->type, "while") == 0)
            {
                if (stack_depth >= MAX_STACK_DEPTH)
                {
                    fprintf (stderr, "Error: stack overflow for while labels\n");
                    exit(1);
                }
                strncpy (stack_labels[stack_depth], tokens[token_count - 1].value, MAX_LENGTH_NAME - 1);
                stack_labels[stack_depth][MAX_LENGTH_NAME - 1] = '\0';
                strncpy (cmd->label, stack_labels[stack_depth], MAX_LENGTH_NAME - 1);
                cmd->label[MAX_LENGTH_NAME - 1] = '\0';
                add_label (cmd->label, command_count);
                stack_depth++;
            }
            else if (strcmp(cmd->type, "end_while") == 0)
            {
                if (stack_depth <= 0)
                {
                    fprintf (stderr, "Error: unmatched end_while at command %d\n", command_count);
                    exit(1);
                }

                stack_depth--;
                strncpy (cmd->label, stack_labels[stack_depth], MAX_LENGTH_NAME - 1);
                cmd->label[MAX_LENGTH_NAME - 1] = '\0';
                add_label (cmd->label, command_count);
            }
            else
            {
                cmd->label[0] = '\0';
            }

            if (strcmp(cmd->type, "store") == 0)
                add_variable (cmd->args[0], (uint64_t) (0x500000 + variable_count * 8), 8);

            command_count++;
        }
    }

    if (command_count >= MAX_COMMANDS)
        fprintf (stderr, "Warning: maximum command limit reached (%d)\n", MAX_COMMANDS);

    int close_err = CloseFile (file);
    if (close_err != NO_ERROR)
        return 1;

    return NO_ERROR;
}

void print_IR_array()
{
    fprintf (stderr, "\n\n\nCommands:\n");

    for (int i = 0; i < command_count; i++)
    {
        IR_Command* cmd = &ir_commands[i];
        fprintf (stderr, PURPLE_TEXT("Command [%02d]: ") BLUE_TEXT("type") "={%-12s} | " BLUE_TEXT("args") "=", i, cmd->type);

        fprintf (stderr, BLUE_TEXT("("));

        for (int j = 0; j < cmd->arg_count; j++)
        {
            fprintf (stderr, "%s", cmd->args[j]);
            if (j < cmd->arg_count - 1) fprintf (stderr, ", ");
        }

        fprintf (stderr, BLUE_TEXT(")"));

        if (cmd->label[0] != '\0')
            fprintf (stderr, " | " BLUE_TEXT("label") "={%s}", cmd->label);

        fprintf (stderr, "\n");
    }

    fprintf (stderr, "\nLabels:\n");
    for (int i = 0; i < label_count; i++)
        fprintf (stderr, PURPLE_TEXT("Label [%02d]:") BLUE_TEXT(" name") "={%s} | " BLUE_TEXT("command_index") "=%d \n",
                i, labels[i].name, labels[i].command_index);

    fprintf (stderr, "\nVariables:\n");
    for (int i = 0; i < variable_count; i++)
        fprintf (stderr, PURPLE_TEXT("Variable [%02d]:") BLUE_TEXT(" name") "={%-10s} | " BLUE_TEXT("address") "=0x%lx | " BLUE_TEXT(
        "size") "=%d \n",
                i, variables[i].name, variables[i].address, variables[i].size);
}

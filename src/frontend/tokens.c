#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "enum.h"
#include "tokens.h"
#include "assert.h"
#include "color_print.h"

int tokenization (struct Context_t* context, const char* string)
{
    int length_string = strlen (string);

    fprintf (stderr, PURPLE_TEXT("\n\nSTART_STRING = <<< %s >>>\n\n"), string);
    int i = 0;

    int old_size      = context->table_size;
    int count_tokens  = 0;

    fprintf (stderr, "old = %d\n", old_size);

    while (string[i] != '$')
    {
        i = skip_spaces (string, length_string, i);

        int start_i = i;

        if ( isalpha(string[i]) )
        {
            // fprintf (stderr, "starting if >>>  start_i = %d\n", start_i);

            while ( isalpha(string[i]) )
                i++;

            int end_i = i;

            // fprintf (stderr, "after skiping spaces >>>  end_i = %d\n", end_i);

            size_t length = end_i - start_i;

            double value = check_keyword (context, &string[start_i], length);

            int num_keyword = find_number_of_keyword (context, &string[start_i], length);

            fprintf (stderr, "POS: %d: num_keywrd = %d" "\n" "value = %lg" "\n", count_tokens, num_keyword, value);

            if (num_keyword > 0)
                fprintf (stderr, "keyword = %d" "\n", context->name_table[num_keyword -1 ].name.is_keyword);
            // fprintf (stderr, "after check_keyword >>>  value = %lg\n", value);
            // fprintf (stderr, "string in this moment >>>  string = '%s'\n\n", &string[start_i]);

            if (value != -1 && num_keyword > 0 && context->name_table[num_keyword - 1].name.is_keyword == 1)
            {
                context->token[count_tokens].type   = OP;
                context->token[count_tokens].value  = value;

                fprintf (stderr, "IN OP >>> context->token[count_tokens].value = %lg\n\n", value);

                count_tokens++;
            }
            else
            {
                if (num_keyword == -1)
                {
                    add_struct_in_keywords (context, &string[start_i], ID, 0, length, 0);

                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = context->table_size;

                    fprintf (stderr, "IN ADD STRUCT_IF >>> context->token[count_tokens].value = %lg\n\n", context->token[count_tokens].value);

                    count_tokens++;
                }
                else
                {
                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = num_keyword;

                    fprintf (stderr, "IN ADD STRUCT >>> context->token[count_tokens].value = %lg\n\n", context->token[count_tokens].value);

                    count_tokens++;
                }
            }

            continue;
        }

        if ( isdigit (string[i]) )
        {
            int val = 0;

            while ('0' <= string[i] && string[i] <= '9')
            {
                val = val * 10 + string[i] - '0';
                i++;
            }

            context->token[count_tokens].type  = NUM;
            context->token[count_tokens].value = val;

            count_tokens++;

            continue;
        }

        if (strchr ("+-*/^()=", string[i]) != NULL)
        {
            int value = check_keyword (context, &string[start_i], 1);

            context->token[count_tokens].type  = OP;
            context->token[count_tokens].value = value;

            count_tokens++;
            i++;
        }
    }

    context->token[count_tokens].type  =  OP;
    context->token[count_tokens].value = '$';

    count_tokens++;

    fprintf (stderr, BLUE_TEXT("\nTOKENS DUMP:\n\n"));

    tokens_dump (context, old_size);

    fprintf (stderr, BLUE_TEXT("\n\nNAME TABLE DUMP:\n\n"));

    name_table_dump (context);

    return 0;
}

int check_keyword (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if ( strncmp (str, context->name_table[i].name.str_pointer, length) == 0 )
            return context->name_table[i].name.code;

    return -1;
}

int find_number_of_keyword (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if ( strncmp (str, context->name_table[i].name.str_pointer, length) == 0 )
            return i + 1;

    return -1;
}

int skip_spaces (const char* string, int length, int current_i)
{
    int i = current_i;

    for (i; i < length; i++)
        if ( !isspace(string[i]) )
            break;

    return i;
}

int tokens_dump (struct Context_t* context, int old_size)
{
    if (context == NULL)
    {
        fprintf (stderr, "context is NULL\n");
        return 1;
    }

    int Id_count = context->table_size - (context->table_size - old_size) + 2;
    int j = 0;

    while (context->token[j].type != 0)
    {
        switch (context->token[j].type)
        {
            case OP:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") "token_type = OP  ||| ADDRESS = [%p] ||| token_value = '%c' (%lg)\n",
                                 j, context[j].token, (int) context->token[j].value, context->token[j].value);
                break;

            case NUM:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") "token_type = NUM ||| ADDRESS = [%p] ||| token_value = %lg\n",
                                 j, context[j].token, context->token[j].value);
                break;

            case ID:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") GREEN_TEXT("token_type = ID  ||| ADDRESS = [%p] ||| token_value = ") BLUE_TEXT("[%lg]\n"),
                                 j, context[j].token, context->token[j].value - 1);

                fprintf (stderr, GREEN_TEXT ("     ADDRESS = [%p], name = '%.*s', length = %zu, is_keyword = %d\n\n"),
                                 context[(int)context->token[j].value - 1].name_table, (int) context->name_table[(int)context->token[j].value - 1].name.length,
                                 context->name_table[(int)context->token[j].value - 1].name.str_pointer,
                                 context->name_table[(int)context->token[j].value - 1].name.length,
                                 context->name_table[(int)context->token[j].value - 1].name.is_keyword);

                Id_count++;

                break;

            default:
                fprintf (stderr, "ERROR in dump: type = %d\n", context->token->type);
        }

        j++;
    }
    return 0;
}

int name_table_dump (struct Context_t* context)
{
    if (context == NULL)
    {
        fprintf (stderr, "context is NULL\n");
        return 1;
    }

    int j = 0;
    while ( context->name_table[j].name.str_pointer != NULL)
    {
        if (context->name_table[j].name.code == 0)
            fprintf (stderr, YELLOW_TEXT("[%.2d]: ADDRESS = [%p], name = '%.*s'\n"),
                             j, context[j].name_table, (int) context->name_table[j].name.length,
                             context->name_table[j].name.str_pointer);
        else
            fprintf (stderr, BLUE_TEXT("[%.2d]: ") "ADDRESS = [%p], name = '%.*s', length = %zu, is_keyword = %d, added_status = %d\n",
                             j, context[j].name_table, (int) context->name_table[j].name.length,
                             context->name_table[j].name.str_pointer, context->name_table[j].name.length,
                             context->name_table[j].name.is_keyword,
                             context->name_table[j].name.added_status);
        j++;
    }

    return 0;
}

int ctor_keywords (struct Context_t* context)
{
    context->table_size = 0;

    add_struct_in_keywords (context,                 "sin",   SIN  , 1, strlen (                "sin"), 0);
    add_struct_in_keywords (context,                 "cos",   COS  , 1, strlen (                "cos"), 0);
    add_struct_in_keywords (context,                  "ln",    LN  , 1, strlen (                 "ln"), 0);
    add_struct_in_keywords (context,                   "+",   ADD  , 1, strlen (                  "+"), 0);
    add_struct_in_keywords (context,                   "-",   SUB  , 1, strlen (                  "-"), 0);
    add_struct_in_keywords (context,                   "*",   MUL  , 1, strlen (                  "*"), 0);
    add_struct_in_keywords (context,                   "/",   DIV  , 1, strlen (                  "/"), 0);
    add_struct_in_keywords (context,                   "^",   POW  , 1, strlen (                  "^"), 0);
    add_struct_in_keywords (context,                   "(",  OP_BR , 1, strlen (                  "("), 0);
    add_struct_in_keywords (context,                   ")",  CL_BR , 1, strlen (                  ")"), 0);
    add_struct_in_keywords (context,             "lesssgo", OP_F_BR, 1, strlen (            "lesssgo"), 0);
    add_struct_in_keywords (context,             "stoopit", CL_F_BR, 1, strlen (            "stoopit"), 0);
    add_struct_in_keywords (context,                  "is",  EQUAL , 1, strlen (                 "is"), 0);
    add_struct_in_keywords (context,             "forreal",   IF   , 1, strlen (            "forreal"), 0);
    add_struct_in_keywords (context,               "money",  WHILE , 1, strlen (              "money"), 0);
    add_struct_in_keywords (context,          "lethimcook",  ADVT  , 1, strlen (         "lethimcook"), 0);
    add_struct_in_keywords (context,              "shutup",  GLUE  , 1, strlen (             "shutup"), 0);
    add_struct_in_keywords (context, "SPACE_FOR_ADDED_OBJ",    0   , 1, strlen ("SPACE_FOR_ADDED_OBJ"), 0);

    return 0;
}

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int is_keyword, int length, int added_status)
{
    context->name_table[context->table_size].name.str_pointer  = str;
    context->name_table[context->table_size].name.code         = code;
    context->name_table[context->table_size].name.is_keyword   = is_keyword;    // YES = 1;
    context->name_table[context->table_size].name.length       = length;
    context->name_table[context->table_size].name.added_status = added_status;

    context->table_size++;

    return 0;
}

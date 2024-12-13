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

    int old_size      = context->table_size - 1;
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

            fprintf (stderr, "POS: %d: num_keywrd = %d\n", count_tokens, num_keyword);

            // fprintf (stderr, "after check_keyword >>>  value = %lg\n", value);
            // fprintf (stderr, "string in this moment >>>  string = '%s'\n\n", &string[start_i]);

            if (value != -1 && context->name_table[count_tokens].name.added == 0)
            {
                context->token[count_tokens].type   = OP;
                context->token[count_tokens].value  = value;

                count_tokens++;
            }
            else
            {
                if (context->name_table[count_tokens].name.added == 0)
                {
                    add_struct_in_keywords (context, &string[start_i], ID, length, 1);

                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = context->table_size;

                    fprintf (stderr, "IN ADD STRUCT >>> context->token[count_tokens].value = %lg\n\n", context->token[count_tokens].value);

                    count_tokens++;
                }
                else
                {
                    int num_keyword = find_number_of_keyword (context, &string[start_i], length);

                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = num_keyword;

                    fprintf (stderr, "IN FILLING TOKEN >>> context->token[count_tokens].value = %lg\n\n", context->token[count_tokens].value);

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

    int Id_count = context->table_size - (context->table_size - old_size);
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
                                 j, context[j].token, context->token[j].value);

                fprintf (stderr, GREEN_TEXT ("     ADDRESS = [%p], name = '%.*s', length = %zu, added_status = %d\n\n"),
                                 context[Id_count].name_table, (int) context->name_table[Id_count].name.length, context->name_table[Id_count].name.str_pointer, context->name_table[Id_count].name.length, context->name_table[Id_count].name.added);

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
        fprintf (stderr, BLUE_TEXT("[%.2d]: ") "ADDRESS = [%p], name = '%.*s', length = %zu, added_status = %d\n",
                         j, context[j].name_table, (int) context->name_table[j].name.length,
                         context->name_table[j].name.str_pointer, context->name_table[j].name.length, context->name_table[j].name.added);
        j++;
    }

    return 0;
}

int ctor_keywords (struct Context_t* context)
{
    context->table_size = 0;

    add_struct_in_keywords (context,      "sin",   SIN  , strlen (     "sin"), 0 );
    add_struct_in_keywords (context,      "cos",   COS  , strlen (     "cos"), 0 );
    add_struct_in_keywords (context,       "ln",    LN  , strlen (      "ln"), 0 );
    add_struct_in_keywords (context,        "+",   ADD  , strlen (       "+"), 0 );
    add_struct_in_keywords (context,        "-",   SUB  , strlen (       "-"), 0 );
    add_struct_in_keywords (context,        "*",   MUL  , strlen (       "*"), 0 );
    add_struct_in_keywords (context,        "/",   DIV  , strlen (       "/"), 0 );
    add_struct_in_keywords (context,        "^",   POW  , strlen (       "^"), 0 );
    add_struct_in_keywords (context,        "(",  OP_BR , strlen (       "("), 0 );
    add_struct_in_keywords (context,        ")",  CL_BR , strlen (       ")"), 0 );
    add_struct_in_keywords (context,  "lesssgo", OP_F_BR, strlen ( "lesssgo"), 0 );
    add_struct_in_keywords (context,  "stoopit", CL_F_BR, strlen ( "stoopit"), 0 );
    add_struct_in_keywords (context,         "=", EQUAL , strlen (       "="), 0 );
    add_struct_in_keywords (context,   "forreal",   IF  , strlen ( "forreal"), 0 );

    return 0;
}

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int length, int add_status)
{
    context->name_table[context->table_size].name.str_pointer = str;
    context->name_table[context->table_size].name.code        = code;
    context->name_table[context->table_size].name.is_keyword  = 1;    // YES = 1;
    context->name_table[context->table_size].name.length      = length;
    context->name_table[context->table_size].name.added       = add_status;

    context->table_size++;

    return 0;
}

#include <stdio.h>
#include <sys/io.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "enum.h"
#include "keywords.h"
#include "tokens.h"
#include "syntax.h"
#include "assert.h"
#include "color.h"

int tokenization (struct Context_t* context, const char* string)
{
    assert (context);
    assert (string);

    int length_string = (int) strlen (string);

    int i = 0;

    int count_tokens  = 0;

    while (string[i] != '$')
    {
        i = skip_spaces (string, length_string, i);

        int start_i = i;

        if ( isalpha(string[i]) )
        {
            while ( isalpha(string[i]) )
                i++;

            int end_i = i;

            int length = end_i - start_i;

            double value = find_name (context, &string[start_i], length);

            int num_keyword = find_number_of_keyword (context, &string[start_i], length);

            if ( (int) value != -1 && num_keyword > 0 && context->name_table[num_keyword].name.is_keyword == 1)
            {
                context->token[count_tokens].type   = OP;
                context->token[count_tokens].value  = value;
                context->token[count_tokens].str    = &string[start_i];

                count_tokens++;
            }
            else
            {
                if (num_keyword == -1)
                {
                    add_struct_in_keywords (context, &string[start_i], (enum Operations) ID, 0, length, 0);

                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = context->table_size - 1; // before 'add_struct' table_size was table_size - 1
                    context->token[count_tokens].str   = &string[start_i];

                    count_tokens++;
                }
                else
                {
                    context->token[count_tokens].type  = ID;
                    context->token[count_tokens].value = num_keyword;
                    context->token[count_tokens].str   = &string[start_i];

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
            context->token[count_tokens].str   = &string[start_i];

            count_tokens++;

            continue;
        }

        if (strchr ("+-*/^()={},<>", string[i]) != NULL)
        {
            int value = find_name (context, &string[start_i], 1);

            context->token[count_tokens].type  = OP;
            context->token[count_tokens].value = value;
            context->token[count_tokens].str   = &string[start_i];

            count_tokens++;
            i++;
        }
    }

    context->token[count_tokens].type  =  OP;
    context->token[count_tokens].value = '$';

    count_tokens++;

    return 0;
}

int find_number_of_keyword (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if ( context->name_table[i].name.length == length &&
             strncmp (str, context->name_table[i].name.str_pointer, (size_t) length) == 0 )
            return i;

    return -1;
}

int skip_spaces (const char* string, int length, int current_i)
{
    int i = current_i;

    while (i < length)
    {
        if ( !isspace(string[i]) )
            break;

        i++;
    }

    return i;
}

int tokens_dump (struct Context_t* context)
{
    if (context == NULL)
    {
        fprintf (stderr, "context is NULL\n");
        return 1;
    }

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

                fprintf (stderr, GREEN_TEXT ("     ADDRESS = [%p], name = '%.*s', length = %d, is_keyword = %d\n\n"),
                                 context[(int)context->token[j].value].name_table,
                                 context->name_table[(int)context->token[j].value].name.length,
                                 context->name_table[(int)context->token[j].value].name.str_pointer,
                                 context->name_table[(int)context->token[j].value].name.length,
                                 context->name_table[(int)context->token[j].value].name.is_keyword);

                break;

            default:
                fprintf (stderr, "ERROR in dump: type = %d\n", context->token->type);
        }

        j++;
    }
    return 0;
}

int name_table_dump (FILE* file, struct Context_t* context)
{
    if (context == NULL)
    {
        fprintf (file, "context is NULL\n");
        return 1;
    }

    // NOTE - istty (fileno(file));

    int j = 0;
    char* str = "";

    if (file != stderr)
    {
        str = ";";

        int i = 0;
        while (i < context->table_size)
        {
            if (context->name_table[i].name.code == SPACE)
                break;

            i++;
        }

        j = i + 1;
    }

    while ( context->name_table[j].name.str_pointer != NULL)
    {
        if (context->name_table[j].name.code == SPACE)
            fprintf (file,  "\n" "%s" YELLOW_TEXT("[%.2d]: ADDRESS = [%p], name = '%.*s'") "\n\n",
                            str, j, context[j].name_table,
                              (int) context->name_table[j].name.length,
                                    context->name_table[j].name.str_pointer);
        else
        {
            if (file == stderr)
                fprintf (file,  "%s" "[%.2d]: " "ADDRESS = [%p], name = '%.*s', length = %d, is_keyword = %d, added_status = %d"
                                "\n" "%s" "id_type = %d, host_func = %d, counter_params = %d, counter_locals = %d, offset = %d\n\n",
                                str, j, context[j].name_table,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.str_pointer,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.is_keyword,
                                        context->name_table[j].name.added_status,
                                str,    context->name_table[j].name.id_type,
                                        context->name_table[j].name.host_func,
                                        context->name_table[j].name.counter_params,
                                        context->name_table[j].name.counter_locals,
                                        context->name_table[j].name.offset);
            else // dump for asm file
                fprintf (file,  "%s" "[%.2d]: " "ADDRESS = [%p], name = '%.*s', %*s lngth = %d, keywrd = %d, added_stts = %d "
                                "id_type = %d, host_fnc = %02d, cntr_prms = %d, cntr_lcls = %d, offset = %d\n",
                                str, j, context[j].name_table,
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.str_pointer,
                                    5 - context->name_table[j].name.length, "",
                                        context->name_table[j].name.length,
                                        context->name_table[j].name.is_keyword,
                                        context->name_table[j].name.added_status,
                                        context->name_table[j].name.id_type,
                                        context->name_table[j].name.host_func,
                                        context->name_table[j].name.counter_params,
                                        context->name_table[j].name.counter_locals,
                                        context->name_table[j].name.offset);
        }

        j++;
    }

    return 0;
}


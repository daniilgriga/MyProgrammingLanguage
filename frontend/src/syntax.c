#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "dsl.h"
#include "log.h"
#include "tree.h"
#include "enum.h"
#include "tokens.h"
#include "syntax.h"
#include "assert.h"

#pragma GCC diagnostic ignored "-Wfloat-equal"
int Position =  0;

#define MOVE_POSITION Position++

// ========================================= GRAMMAR ========================================= //
//    Grammar                   ::= { FunctionDef }* '$'
//
//    CompoundParametersForCall ::= { Expression, }*
//    CompoundParametersForDef  ::= { 'lethimcook' Ident, }*
//    Compound_Operator         ::= 'lesssgo' { { Assignment | Cond | Cycle } 'shutup' }+ 'stoopit'
//    Cond                      ::= 'forreal' '('Expression')' Compound_Operator
//    Cycle                     ::= 'grinding' '('Expression')' Compound_Operator
//
//    FunctionDef               ::= 'lethimcook' Ident '(' CompoundParametersForDef ')' Compound_Operator
//    FunctionCall              ::= Ident '(' CompoundParametersForCall ')'
//
//    Assignment                ::= 'lethimcook'? Ident '=' Expression
//    Expression                ::= Term { ['+''-'] Term }*
//    Term                      ::= |sqrt| P    { ['*''/''sqrt'] P    }*
//    P                         ::= '('Expression')' | Ident | Number | FunctionCall
//
//    Ident                     ::= ['a'-'z']+
//    Number                    ::= ['0'-'9']+
//

static struct Node_t* GetFunctionCall           (struct Context_t* context);
static struct Node_t* GetFunctionDef            (struct Context_t* context);
static struct Node_t* GetAssignment             (struct Context_t* context);
static struct Node_t* GetComparison             (struct Context_t* context);
static struct Node_t* GetExpression             (struct Context_t* context);
static struct Node_t* GetOperation              (struct Context_t* context);
static struct Node_t* GetNumber                 (struct Context_t* context);
static struct Node_t* GetIdent                  (struct Context_t* context);
static struct Node_t* GetCond                   (struct Context_t* context);
static struct Node_t* GetLoop                   (struct Context_t* context);
static struct Node_t* GetTerm                   (struct Context_t* context);
static struct Node_t* GetP                      (struct Context_t* context);

static struct Node_t* CompoundParametersForCall (struct Context_t* context);
static struct Node_t* CompoundParametersForDef  (struct Context_t* context);
static struct Node_t* CompoundOperations        (struct Context_t* context);

void dump_token (struct Context_t* context, int numb_of_token);

void local_variable_offset_counter (struct Context_t* context);

//======================== DSL FOR CURRENT TOKEN && NAME TABLE ACCESS =======================//

#define _CUR_TOKEN  ( context->token[Position]     )
#define _NEXT_TOKEN ( context->token[Position + 1] )

#define _IS_OP(val) ( _CUR_TOKEN.type  == OP && \
                      _CUR_TOKEN.value == (val) )

#define _CUR_NAME   ( context->name_table[ (int) _CUR_TOKEN.value ].name )

// CURR.type ; CURR.name ????

//===========================================================================================//

struct Node_t* GetGrammar (struct Context_t* context)
{
    struct Node_t* node = GetFunctionDef (context);

    struct Node_t* root = _DEFGL (node, NULL) ;

    struct Node_t* link = root;

    while ( _IS_OP (ADVT) )
    {
        node = GetFunctionDef (context);

        struct Node_t* right_node = _DEFGL (node, NULL);

        link->right = right_node;

        link = right_node;
    }

    if ( !_IS_OP('$') )
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_END_OF_FILE);

    MOVE_POSITION;

    local_variable_offset_counter (context);

    return root;
}

static struct Node_t* GetFunctionDef (struct Context_t* context)
{
    if ( _IS_OP (ADVT) )
    {
        MOVE_POSITION;

        _CUR_NAME.added_status = 1;
    }
    else
    {
        if ( _CUR_TOKEN.type == ID && _CUR_NAME.added_status == 0 )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);
    }

    struct Node_t* node       = NULL;
    struct Node_t* node_param = NULL;

    // current Position is token with function name
    if ( _CUR_TOKEN.type   == ID &&
         _NEXT_TOKEN.value == OP_BR )
    {
        node = _FUNC (_CUR_TOKEN.value);

        context->curr_host_func = (int) node->value;

        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            node_param = CompoundParametersForDef (context);


            if ( !_IS_OP (CL_BR) )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE);

            MOVE_POSITION;
        }
        else
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_OPEN_BRACE);

        node = _CALL (node, node_param);
    }

    struct Node_t* func_name = node;

    struct Node_t* func_body = CompoundOperations (context);

    return _DEF (func_name, func_body);
}

void local_variable_offset_counter (struct Context_t* context)
{
    int count = 0;

    for (int i = 0; i < context->table_size; i++)
    {
        if (context->name_table[i].name.is_keyword == 0 && context->name_table[i].name.id_type == 0)
        {
            for (int j = i; j < context->table_size; j++)
            {
                if ( context->name_table[j].name.id_type == 6 ||                                       // 20 - name table code of 'main' func
                   ( context->name_table[j].name.id_type == 5 && context->name_table[j].name.host_func != 20 ) )
                {
                    context->name_table[j].name.offset = count;
                    count++;
                }
            }

            count = 0;
        }
    }
}

static struct Node_t* GetFunctionCall (struct Context_t* context)
{
    struct Node_t* node       = NULL;
    struct Node_t* node_param = NULL;

    if ( _CUR_TOKEN.type   == ID &&
         _NEXT_TOKEN.value == OP_BR )
    {
        node = _FUNC (_CUR_TOKEN.value);

        MOVE_POSITION;
        MOVE_POSITION;

        node_param = CompoundParametersForCall (context);

        node = _CALL (node, node_param);

        MOVE_POSITION;
    }

    return node;
}

struct Node_t* GetOperation (struct Context_t* context)
{
    struct Node_t*    node = GetAssignment (context);

    if (node == NULL) node = GetCond (context);

    if (node == NULL) node = GetLoop (context);

    if (node == NULL) node = GetExpression (context);

    dump_token (context, 0);

    if (node != NULL)
    {
        if ( _IS_OP (GLUE) ) MOVE_POSITION;
        else                 SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_GLUE_MARK);
    }

    return node;
}

struct Node_t* GetAssignment (struct Context_t* context)
{
    fprintf (stderr, "\nin GetA starting (Pos = %d): cur: type = %d, value = %lg" "\n" "next: type = %d, value = %lg\n\n", Position,
                       _CUR_TOKEN.type,  _CUR_TOKEN.value,
                      _NEXT_TOKEN.type, _NEXT_TOKEN.value );

    struct Node_t* val_1 = NULL;

    if ( _NEXT_TOKEN.value != OP_BR )
    {
        if ( _IS_OP (ADVT) )
        {
            MOVE_POSITION;

            _CUR_NAME.added_status = 1;

            val_1 = GetIdent (context);

            context->name_table[(int)val_1->value].name.id_type   = LOCL;
            context->name_table[(int)val_1->value].name.host_func = context->curr_host_func;
            context->name_table[context->curr_host_func].name.counter_locals++;

            fprintf (stderr, "\nvalue = %lg", val_1->value);
            fprintf (stderr, "\nname = '%s'\n", context->name_table[context->name_table[(int)val_1->value].name.host_func].name.str_pointer);
        }
        else
        {
            fprintf (stderr, "\nPosition = %d; token_value = %lg >>> added_status = %d, is_keyword = %d\n", Position, _CUR_TOKEN.value, _CUR_NAME.added_status, _CUR_NAME.is_keyword);

            if ( ( _CUR_TOKEN.type        == ID &&
                   _CUR_NAME.added_status == 0 ) ||
                   _CUR_NAME.host_func    != context->curr_host_func )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);

            val_1 = GetIdent (context);
        }

        while ( _IS_OP (EQUAL) )
        {
            MOVE_POSITION;

            struct Node_t* val_2 = GetExpression (context);

            val_1 = _EQL (val_1, val_2);
        }

        return val_1;
    }

    return NULL;
}

static struct Node_t* GetExpression (struct Context_t* context)
{
    struct Node_t* val = GetTerm (context);

    dump_in_log_file (val, context, "BEFORE 'WHILE' IN GetExpression >>> LEFT NODE:");

    while ( _IS_OP (ADD) || _IS_OP (SUB) )
    {
        int op = (int) _CUR_TOKEN.value;

        MOVE_POSITION;

        struct Node_t* val2 = GetTerm (context);

        dump_in_log_file (val2, context, "IN 'WHILE' IN GetExpression >>> RIGHT NODE");

        if (op == ADD)
        {
            val = _ADD (val, val2);
            dump_in_log_file (val, context, "FRESH NODE AHAHAH:");
        }
        else
            val = _SUB (val, val2);
    }

    return val;
}

static struct Node_t* GetTerm (struct Context_t* context)
{
    struct Node_t* val = GetP (context);

    while ( _IS_OP (MUL) || _IS_OP (DIV) )
    {
        int op = (int) _CUR_TOKEN.value;

        MOVE_POSITION;

        struct Node_t* val2 = GetP (context);

        if (op == MUL)
            val = _MUL (val, val2);
        else
            val = _DIV (val, val2);
    }

    return val;
}

static struct Node_t* GetP (struct Context_t* context)
{
    if ( _IS_OP (OP_BR) )
    {
        MOVE_POSITION;

        struct Node_t* val = GetExpression (context);

        if ( !_IS_OP (CL_BR) )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE);

        MOVE_POSITION;

        return val;
    }
    else
    {
        struct Node_t* node_ID = GetIdent (context);

        if (node_ID != NULL)
            return node_ID;
        else
        {
            struct Node_t* node_F = GetFunctionCall (context);

            log_printf ("\n" "Im IN GetP" "\n" "CURRENT TOKEN TYPE = %d, VALUE = '%c' (%lg)",
                         _CUR_TOKEN.type, (int)_CUR_TOKEN.value, _CUR_TOKEN.value);

            if (node_F != NULL)
                return node_F;
            else
                return GetNumber (context);
        }
    }
}

static struct Node_t* GetIdent (struct Context_t* context)
{
    struct Node_t* node = NULL;

    if ( _CUR_TOKEN.type   == ID &&
         _NEXT_TOKEN.value != OP_BR)
    {
        if (_CUR_NAME.added_status == 0)
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);

        node = _ID (_CUR_TOKEN.value);

        fprintf (stderr, "\nnode [%p]: node->value = %lg\n", node, node->value);

        MOVE_POSITION;
    }

    return node;
}

static struct Node_t* GetNumber (struct Context_t* context)
{
    if ( _CUR_TOKEN.type == NUM )
    {
        struct Node_t* node = _NUM (_CUR_TOKEN.value);

        MOVE_POSITION;

        return node;
    }

    return NULL;
}

static struct Node_t* GetComparison (struct Context_t* context)
{
    struct Node_t* val = GetExpression (context);

    if ( _IS_OP (GT) || _IS_OP (LT) || _IS_OP (GTE) || _IS_OP (NEQ) || _IS_OP (EQ) )
    {
        int op = (int) _CUR_TOKEN.value;

        MOVE_POSITION;

        struct Node_t* val2 = GetExpression (context);

        if      (op == GT)  val = _GT  (val, val2);
        else if (op == LT)  val = _LT  (val, val2);
        else if (op == GTE) val = _GTE (val, val2);
        else if (op == NEQ) val = _NEQ (val, val2);
        else                val = _EQ  (val, val2);
    }

    return val;
}

static struct Node_t* GetCond (struct Context_t* context)
{
    struct Node_t* GetE = NULL;
    struct Node_t* GetA = NULL;

    if ( _IS_OP (IF) )
    {
        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            GetE = GetComparison (context);

            if ( !_IS_OP (CL_BR) )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE);

            MOVE_POSITION;

            GetA = CompoundOperations (context);

            return _IF (GetE, GetA);
        }
        else
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_OPEN_BRACE);
    }
    else
        return NULL;
}

static struct Node_t* GetLoop (struct Context_t* context)
{
    struct Node_t* GetE = NULL;
    struct Node_t* GetA = NULL;

    if ( _IS_OP (WHILE) )
    {
        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            GetE = GetComparison (context);

            if ( !_IS_OP (CL_BR) )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE);

            MOVE_POSITION;

            GetA = CompoundOperations (context);

            return _WHILE (GetE, GetA);

        }
        else
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_OPEN_BRACE);
    }
    else
        return NULL;
}

static struct Node_t* CompoundOperations (struct Context_t* context)
{
    if ( _IS_OP (OP_F_BR) )
    {
        MOVE_POSITION;

        struct Node_t* node = GetOperation (context);

        struct Node_t* root = _OP (node, NULL) ;

        struct Node_t* link = root;

        while ( _CUR_TOKEN.type  == ID || _IS_OP (ADVT) || _IS_OP (IF) || _IS_OP (WHILE) )
        {
            node = GetOperation (context);

            struct Node_t* right_node = _OP  (node, NULL);

            link->right = right_node;

            link = right_node;
        }

        if ( !_IS_OP (CL_F_BR) )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE_OF_COND_OP);

        MOVE_POSITION;

        return root;
    }
    else
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_OPEN_BRACE_OF_COND_OP);
}

static struct Node_t* CompoundParametersForCall (struct Context_t* context)
{
    struct Node_t* node = GetExpression (context);

    struct Node_t* root = _PRM (node, NULL);

    struct Node_t* link = root;

    if ( _IS_OP (COMMA) )
    {
        MOVE_POSITION;

        while ( _CUR_TOKEN.type  == ID || _CUR_TOKEN.type  == NUM )
        {
            node = GetExpression (context);

            struct Node_t* right_node = _PRM  (node, NULL);

            link->right = right_node;

            link = right_node;

            if ( !_IS_OP (COMMA) )
                break;

            MOVE_POSITION;
        }
    }

    return root;
}

static struct Node_t* CompoundParametersForDef (struct Context_t* context)
{
    if ( _IS_OP (ADVT) )
    {
        MOVE_POSITION;

        _CUR_NAME.added_status = 1;
    }
    else
    {
        fprintf (stderr, "\nPosition = %d; token_value = %lg >>> added_status = %d, is_keyword = %d\n", Position, _CUR_TOKEN.value, _CUR_NAME.added_status, _CUR_NAME.is_keyword);

        if ( _CUR_TOKEN.type == ID &&
             _CUR_NAME.added_status == 0 )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);
    }

    struct Node_t* node = GetIdent (context);

    if (context->name_table[(int)node->value].name.id_type == LOCL)
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);

    context->name_table[(int)node->value].name.id_type   = PARM;
    context->name_table[(int)node->value].name.host_func = context->curr_host_func;
    context->name_table[context->curr_host_func].name.counter_params++;

    struct Node_t* root = _PRM (node, NULL);

    struct Node_t* link = root;

    if ( _IS_OP (COMMA) )
    {
        MOVE_POSITION;

        while ( _IS_OP (ADVT) && _NEXT_TOKEN.type  == ID)
        {
            MOVE_POSITION;

            _CUR_NAME.added_status = 1;

            dump_token (context, 0);

            node = GetIdent (context);

            context->name_table[(int)node->value].name.id_type   = PARM;
            context->name_table[(int)node->value].name.host_func = context->curr_host_func;
            context->name_table[context->curr_host_func].name.counter_params++;

            struct Node_t* right_node = _PRM  (node, NULL);

            link->right = right_node;

            link = right_node;

            if ( !_IS_OP (COMMA) )
                break;

            MOVE_POSITION;
        }
    }

    dump_in_log_file (root, context, "FRESH NODE IS REEEEAAADYYYYYY:\n");

    return root;
}

[[noreturn]] void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line, int error)
{
    fprintf (stderr, "\n" PURPLE_TEXT("%s: %s:%d: ") RED_TEXT("SYNTAX ERROR (code = %d) in %d position: "), filename, func, line, error, Position);

    switch (error)
    {
        case NOT_FIND_END_OF_FILE:
            fprintf (stderr, "expected " WHITE_TEXT("'$'") " after %s\n", context->token[Position - 1].str);

            break;

        case NOT_FIND_GLUE_MARK:
            if (context->token[Position - 1].type == NUM)
                fprintf (stderr, "expected " WHITE_TEXT("'shutup'") " after " WHITE_TEXT("'%lg'")   "\n",       context->token[Position - 1].value);
            if (context->token[Position - 1].type == OP)
                fprintf (stderr, "expected " WHITE_TEXT("'shutup'") " after " WHITE_TEXT("'%c'")    "\n", (int) context->token[Position - 1].value);
            if (context->token[Position - 1].type == ID)
                fprintf (stderr, "expected " WHITE_TEXT("'shutup'") " after " WHITE_TEXT("'%.*s'")  "\n",
                                 (int) context->name_table[(int)context->token[Position - 1].value].name.length,
                                       context->name_table[(int)context->token[Position - 1].value].name.str_pointer);

            break;

        case UNDECLARED:
            fprintf (stderr, "undeclared " WHITE_TEXT("'%.*s'")  "\n",
                             (int) context->name_table[(int)_CUR_TOKEN.value].name.length,
                                   context->name_table[(int)_CUR_TOKEN.value].name.str_pointer);

            break;

        case NOT_FIND_OPEN_BRACE:
            fprintf (stderr, "expected " WHITE_TEXT("'('") " before " WHITE_TEXT("'%s'")  "\n",
                             ( (int) context->token[Position - 1].value == 'w') ? "grinding" : "forreal" );

            break;

        case NOT_FIND_MAIN_FUNC:
            fprintf (stderr, "expected main function\n");

            break;

        default:
            fprintf (stderr, "Unknown error\n");
    }

    exit (1);
}

void dump_token (struct Context_t* context, int numb_of_token)
{
    fprintf (stderr, "\n" "Token number %d: token type = %d, token_value = '%c' (%lg), str = '%.20s'" "\n",
                      Position + numb_of_token, context->token[Position + numb_of_token].type,
                      (int) context->token[Position + numb_of_token].value, context->token[Position + numb_of_token].value,
                            context->token[Position + numb_of_token].str );

    log_printf ("\n" "Token number %d: token type = %d, token_value = '%c' (%lg), str = '%.20s'" "\n",
                      Position + numb_of_token, context->token[Position + numb_of_token].type,
                      (int) context->token[Position + numb_of_token].value, context->token[Position + numb_of_token].value,
                            context->token[Position + numb_of_token].str );
}

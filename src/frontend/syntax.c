#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "enum.h"
#include "dsl.h"
#include "tokens.h"
#include "syntax.h"
#include "assert.h"
#include "color_print.h"

int Position =  0;

#define MOVE_POSITION Position++

/*========================================= GRAMMAR ========================================= */
/*    Grammar           ::= FunctionDef+ '$'
 *
 *    Compound_Operator ::= 'lesssgo' { { Assignment | Cond | Cycle } 'shutup' }+ 'stoopit'
 *    Cond              ::= 'forreal' '('Expression')' Compound_Operator
 *    Cycle             ::= 'money'   '('Expression')' Compound_Operator
 *
 *    FunctionDef       ::= 'lethimcook' Ident '(' ')' Compound_Operator
 *    FunctionCall      ::= Ident '(' ')'
 *
 *    Assignment        ::= 'lethimcook' Ident '=' Expression
 *    Expression        ::= Term { ['+''-'] Term }*
 *    Term              ::= P    { ['*''/'] P    }*
 *    Pow               ::= P    { ['^'] P       }*
 *    P                 ::= '('Expression')' | Ident | Number | FunctionCall
 *
 *
 *    Ident            ::= ['a'-'z']+
 *    Number           ::= ['0'-'9']+
 */

static struct Node_t*  GetAssignment   (struct Context_t* context);
static struct Node_t*  GetExpression   (struct Context_t* context);
static struct Node_t*  GetOperation    (struct Context_t* context);
static struct Node_t*  GetFunctionDef  (struct Context_t* context);
static struct Node_t*  GetFunctionCall (struct Context_t* context);
static struct Node_t*  GetNumber       (struct Context_t* context);
static struct Node_t*  GetIdent        (struct Context_t* context);
static struct Node_t*  GetCond         (struct Context_t* context);
static struct Node_t*  GetLoop         (struct Context_t* context);
static struct Node_t*  GetTerm         (struct Context_t* context);
static struct Node_t*  GetPow          (struct Context_t* context);
static struct Node_t*  GetP            (struct Context_t* context);

struct Node_t* CompoundOperations (struct Context_t* context);

static void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line, int error);

#define _IS_OP(val)  ( context->token[Position].type  == OP && \
                       context->token[Position].value == (val) )

struct Node_t* GetGrammar (struct Context_t* context)
{
    //if ( _IS_OP (ADVT) )
    //{


    struct Node_t* root = GetFunctionDef (context);

    //}
    //else
    //    SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);

    if ( !_IS_OP('$') )
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_END_OF_FILE);

    MOVE_POSITION;

    return root;
}

static struct Node_t*  GetFunctionDef  (struct Context_t* context)
{
    if ( _IS_OP (ADVT) )
    {
        MOVE_POSITION;

        context->name_table[ (int) context->token[Position].value ].name.added_status = 1;
    }
    else
    {
        fprintf (stderr, "\nPosition = %d; token_value = %lg >>> added_status = %d, is_keyword = %d\n", Position, context->token[Position].value, context->name_table[ (int) context->token[Position].value ].name.added_status, context->name_table[ (int) context->token[Position].value ].name.is_keyword);

        if ( context->token[Position].type == ID &&
                context->name_table[ (int) context->token[Position].value ].name.added_status == 0 )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);
    }

    struct Node_t* func_name = GetFunctionCall (context);

    struct Node_t* func_body = CompoundOperations (context);

    return _DEF (func_name, func_body);
}

static struct Node_t*  GetFunctionCall (struct Context_t* context)
{
    struct Node_t* node = NULL;

    if (context->token[Position].type == ID && context->token[Position + 1].value == OP_BR)
    {
        node = _FUNC (context->token[Position].value);

        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            if ( !_IS_OP (CL_BR) )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_CLOSE_BRACE);

            MOVE_POSITION;
        }
        else
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_OPEN_BRACE);
    }

    return node;
}

struct Node_t* GetOperation  (struct Context_t* context)
{
    struct Node_t*    node = GetAssignment (context);

    if (node == NULL) node = GetCond (context);

    if (node == NULL) node = GetLoop (context);

    if (node == NULL) node = GetExpression (context);

    if (node != NULL)
        if ( _IS_OP (GLUE) ) MOVE_POSITION;
        else                 SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, NOT_FIND_GLUE_MARK);

    return node;
}

struct Node_t* GetAssignment (struct Context_t* context)
{
    fprintf (stderr, "\nin GetA starting (Pos = %d): cur: type = %d, value = %lg" "\n" "next: type = %d, value = %lg\n\n", Position,
                      context->token[Position].type, context->token[Position].value,
                      context->token[Position+1].type, context->token[Position+1].value );

    if (context->token[Position + 1].value != OP_BR)
    {
        if ( _IS_OP (ADVT) )
        {
            MOVE_POSITION;

            context->name_table[ (int) context->token[Position].value ].name.added_status = 1;
        }
        else
        {
            fprintf (stderr, "\nPosition = %d; token_value = %lg >>> added_status = %d, is_keyword = %d\n", Position, context->token[Position].value, context->name_table[ (int) context->token[Position].value ].name.added_status, context->name_table[ (int) context->token[Position].value ].name.is_keyword);

            if ( context->token[Position].type == ID &&
                 context->name_table[ (int) context->token[Position].value ].name.added_status == 0 )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);
        }

        struct Node_t* val_1 = GetIdent (context);

        while ( _IS_OP (EQUAL) )
        {
            fprintf (stderr, "im in GetA: pos = %d\n", Position);

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

    while ( _IS_OP (ADD) || _IS_OP (SUB) )
    {
        int op = context->token[Position].value;

        MOVE_POSITION;

        struct Node_t* val2 = GetTerm (context);

        if (op == ADD)
            val = _ADD (val, val2);
        else
            val = _SUB (val, val2);
    }

    return val;
}

static struct Node_t* GetTerm (struct Context_t* context)
{
    struct Node_t* val = GetPow (context);

    while ( _IS_OP (MUL) || _IS_OP (DIV) )
    {
        int op = context->token[Position].value;

        MOVE_POSITION;

        struct Node_t* val2 = GetPow (context);

        if (op == MUL)
            val = _MUL (val, val2);
        else
            val = _DIV (val, val2);
    }

    return val;
}

static struct Node_t* GetPow (struct Context_t* context)
{
    struct Node_t* node = GetP (context);

    while ( _IS_OP (POW) )
    {
        MOVE_POSITION;

        struct Node_t* exp = GetP (context);

        node = _POW (node, exp);
    }

    return node;
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

    if ( context->token[Position].type  == ID &&
         context->token[Position + 1].value != OP_BR)
    {
        if (context->name_table[ (int) context->token[Position].value ].name.added_status == 0)
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__, UNDECLARED);

        node = _ID (context->token[Position].value);

        fprintf (stderr, "\nnode [%p]: node->value = %lg\n", node, node->value);

        MOVE_POSITION;
    }

    return node;
}

static struct Node_t* GetNumber (struct Context_t* context)
{
    if ( context->token[Position].type == NUM )
    {
        struct Node_t* node = _NUM (context->token[Position].value);

        MOVE_POSITION;

        return node;
    }
}

static struct Node_t*  GetCond (struct Context_t* context)
{
    struct Node_t* GetE = NULL;
    struct Node_t* GetA = NULL;

    if ( _IS_OP (IF) )
    {
        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            GetE = GetExpression (context);

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

static struct Node_t*  GetLoop (struct Context_t* context)
{
    struct Node_t* GetE = NULL;
    struct Node_t* GetA = NULL;

    if ( _IS_OP (WHILE) )
    {
        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            MOVE_POSITION;

            GetE = GetExpression (context);

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

struct Node_t* CompoundOperations (struct Context_t* context)
{
    if ( _IS_OP (OP_F_BR) )
    {
        MOVE_POSITION;

        struct Node_t* node = GetOperation (context);

        struct Node_t* root = _OP (node, NULL) ;

        struct Node_t* link = root;

        while ( context->token[Position].type  == ID || _IS_OP (ADVT) || _IS_OP (IF) || _IS_OP (WHILE) )
        {
            struct Node_t* node = GetOperation (context);

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

static void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line, int error)
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
                                 (int) context->name_table[(int)context->token[Position - 1].value - 1].name.length,
                                       context->name_table[(int)context->token[Position - 1].value - 1].name.str_pointer);

            break;

        case UNDECLARED:
            fprintf (stderr, "undeclared " WHITE_TEXT("'%.*s'")  "\n",
                             (int) context->name_table[(int)context->token[Position].value - 1].name.length,
                                   context->name_table[(int)context->token[Position].value - 1].name.str_pointer);

            break;

        case NOT_FIND_OPEN_BRACE:
            fprintf (stderr, "expected " WHITE_TEXT("'('") " before " WHITE_TEXT("'%s'")  "\n",
                             ( (int) context->token[Position - 1].value == 'w') ? "money" : "forreal" );

            break;

        case NOT_FIND_MAIN_FUNC:
            fprintf (stderr, "expected main function\n");

            break;

        default:
            fprintf (stderr, "Unknown error\n");
    }

    exit (1);
}

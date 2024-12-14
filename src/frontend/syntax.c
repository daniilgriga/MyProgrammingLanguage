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

/*         GRAMMAR
 *    G ::= {OP}* $
 *   OP ::= A | S
 *    S ::= 'forreal' '('E')' '{' {OP}* '}'
 *
 *    A ::= Id '=' E
 *    E ::= T {['+''-']T}*
 *    T ::= P {['*''/']P}*
 *    P ::= '('E')' | Id | N | Pow
 *  Pow ::= P {['^']P}*
 *
 *
 *   Id ::= ['a'-'z']+
 *    N ::= ['0'-'9']+
 */

static struct Node_t*  GetOperation  (struct Context_t* context);
static struct Node_t*  GetAssignment (struct Context_t* context);
static struct Node_t*  GetExpression (struct Context_t* context);
static struct Node_t*  GetTerm       (struct Context_t* context);
static struct Node_t*  GetP          (struct Context_t* context);
static struct Node_t*  GetNumber     (struct Context_t* context);
static struct Node_t*  GetIdent      (struct Context_t* context);
static struct Node_t*  GetState      (struct Context_t* context);

static struct Node_t*  GetFunc       (struct Context_t* context);
static struct Node_t*  GetPow        (struct Context_t* context);

struct Node_t* union_of_operations (struct Context_t* context);

static void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line);

#define _IS_OP(val)  ( context->token[Position].type  == OP && \
                       context->token[Position].value == (val) )

struct Node_t* GetGrammar (struct Context_t* context)
{
    struct Node_t* node = GetOperation (context);

    int count = 0;

    struct Node_t* root = _OP (node, NULL) ;

    struct Node_t* link = root;

    dump_in_log_file (root, "GetGrammar before while:");

    while ( context->token[Position].type  == ID || _IS_OP (IF) ) // inf cycle
    {
        struct Node_t* node = GetOperation (context);

        struct Node_t* right_node = _OP  (node, NULL);

        link->right = right_node;

        link = right_node;

        dump_in_log_file (root, "GetGrammar after while N%d." "\n" "next operation: '%c'", count, (int) context->token[Position].value);

        count++;
    }

    if ( !_IS_OP('$') )
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

    MOVE_POSITION;

    return root;
}

struct Node_t* GetOperation  (struct Context_t* context)
{
    struct Node_t* node_A = GetAssignment (context);

    if (node_A != NULL)
        return node_A;
    else
        return GetState (context);
}

struct Node_t* GetAssignment (struct Context_t* context)
{
    struct Node_t* val_1 = GetIdent (context);

    while ( _IS_OP (EQUAL) )
    {
        fprintf (stderr, "im in GetA in if\n");

        MOVE_POSITION;

        struct Node_t* val_2 = GetExpression (context);

        val_1 = _EQL (val_1, val_2);
    }

    return val_1;
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
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

        MOVE_POSITION;

        return val;
    }
    else
    {
        fprintf (stderr, "\n" "Position = %d" "\n", Position);

        struct Node_t* node_ID = GetIdent (context);

        if (node_ID != NULL)
            return node_ID;
        else
        {
            struct Node_t* node_F = GetFunc (context);

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

    if ( context->token[Position].type  == ID )
    {
        node = _ID (context->token[Position].value);

        fprintf (stderr, "\nnode [%p]: node->value = %lg\n", node, node->value);

        MOVE_POSITION;
    }

    return node;
}

static struct Node_t* GetNumber (struct Context_t* context)
{
    if (context->token[Position].type == NUM)
    {
        struct Node_t* node = _NUM (context->token[Position].value);

        MOVE_POSITION;

        return node;
    }
}

static struct Node_t* GetFunc (struct Context_t* context)
{
    int func = 0;

    if      ( context->token[Position].value == COS ) func = COS;
    else if ( context->token[Position].value == SIN ) func = SIN;
    else return NULL;

    MOVE_POSITION;

    struct Node_t* node_E = NULL;

    if ( _IS_OP (OP_BR) )
    {
        MOVE_POSITION;

        node_E = GetExpression (context);

        if ( !_IS_OP (CL_BR))
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

        MOVE_POSITION;

        struct Node_t* node_F = NULL;

        if (func == COS) node_F = _COS (node_E);
        if (func == SIN) node_F = _SIN (node_E);

        return node_F;
    }
    else
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);
}

static struct Node_t*  GetState (struct Context_t* context)
{
    struct Node_t* GetE = NULL;
    struct Node_t* GetA = NULL;

    if ( _IS_OP (IF) )
    {
        fprintf (stderr, "POS = %d: IN OP IF:\n", Position);

        MOVE_POSITION;

        if ( _IS_OP (OP_BR) )
        {
            fprintf (stderr, "POS = %d: IN OP_BR IF:\n", Position);

            MOVE_POSITION;

            GetE = GetExpression (context);

            if ( !_IS_OP (CL_BR) )
                SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

            MOVE_POSITION;

            GetA = union_of_operations (context);

            return _IF (GetE, GetA);

        }
        else
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);
    }
    else
    {

        fprintf (stderr, "IN ELSE IF:\n");

        MOVE_POSITION;

        return NULL;
    }
}

struct Node_t* union_of_operations (struct Context_t* context)
{
    if ( _IS_OP (OP_F_BR) )
    {
        MOVE_POSITION;

        struct Node_t* node = GetOperation (context);

        struct Node_t* root = _OP (node, NULL) ;

        struct Node_t* link = root;

        while ( context->token[Position].type  == ID)
        {
            struct Node_t* node = GetOperation (context);

            struct Node_t* right_node = _OP  (node, NULL);

            link->right = right_node;

            link = right_node;
        }

        if ( !_IS_OP (CL_F_BR) )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

        MOVE_POSITION;

        return root;
    }
    else
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);
}

static void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line)
{
    fprintf (stderr, "\n" PURPLE_TEXT("%s: %s:%d: ") RED_TEXT("ERROR in read >>> SYNTAX-ERROR ") "Position = %d: ""SYMBOL = '%c' (%lg)" "\n\n",
                     filename, func, line, Position, (int) context->token[Position].value, context->token[Position].value);

    exit (1);
}

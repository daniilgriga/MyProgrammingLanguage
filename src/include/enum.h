#pragma once

enum Type
{
    NUM  = 1,
    OP   = 2,
    ID   = 3,
    FUNC = 4,
    ROOT = -1
};

enum Operations
{
    ADD     = '+',
    SUB     = '-',
    DIV     = '/',
    MUL     = '*',
    SIN     = 's',
    COS     = 'c',
    POW     = '^',
    LN      = 'l',
    OP_BR   = '(',
    CL_BR   = ')',
    OP_F_BR = '{',
    CL_F_BR = '}',
    EQUAL   = '=',
    IF      = 'i',
    WHILE   = 'w',
    ADVT    = 'A',
    FUNCTION = 'F',
    GLUE    = ';'
};

enum Errors
{
    NOT_FIND_END_OF_FILE            =   1,
    NOT_FIND_GLUE_MARK              =   2,
    UNDECLARED                  =   4,
    NOT_FIND_OPEN_BRACE             =   8,
    NOT_FIND_CLOSE_BRACE            =  16,
    NOT_FIND_OPEN_BRACE_OF_COND_OP  =  32,
    NOT_FIND_CLOSE_BRACE_OF_COND_OP =  64,
    NOT_FIND_MAIN_FUNC              = 128
};

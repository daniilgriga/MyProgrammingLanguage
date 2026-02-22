#pragma once

enum Type
{
    NUM  = 1,
    OP   = 2,
    ID   = 3,
    FUNC = 4,
    PARM = 5,
    LOCL = 6,
    ROOT =-1
};

enum Operations
{
    SPACE   =  0 ,
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
    CALL    = 'C',
    DEF     = 'D',
    COMMA   = ',',
    GLUE    = ';',
    FN_GLUE = '@'
};

#pragma once

enum Type
{
    NUM  = 1,
    OP   = 2,
    VAR  = 3,
    ID   = 4,
    ROOT = -1
};

enum Operations
{
    ADD   = '+',
    SUB   = '-',
    DIV   = '/',
    MUL   = '*',
    SIN   = 's',
    COS   = 'c',
    POW   = '^',
    LN    = 'l',
    OP_BR = '(',
    CL_BR = ')',
    EQUAL = '='
};

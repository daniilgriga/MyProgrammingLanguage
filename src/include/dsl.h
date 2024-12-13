#pragma once

#define _ADD(left, right) new_node (  OP,     ADD, (left), (right) )
#define _SUB(left, right) new_node (  OP,     SUB, (left), (right) )
#define _MUL(left, right) new_node (  OP,     MUL, (left), (right) )
#define _POW(left, right) new_node (  OP,     POW, (left), (right) )
#define _DIV(left, right) new_node (  OP,     DIV, (left), (right) )
#define _SIN(arg)         new_node (  OP,     SIN,  (arg),    NULL )
#define _COS(arg)         new_node (  OP,     COS,  (arg),    NULL )
#define _LN(arg)          new_node (  OP,      LN,   NULL,   (arg) )
#define _NUM(value)       new_node ( NUM, (value),   NULL,    NULL )
#define _ID(value)        new_node (  ID, (value),   NULL,    NULL )
#define _EQL(left, right) new_node (  OP,   EQUAL, (left), (right) )
#define _IF(left, right)  new_node (  OP,      IF, (left), (right) )

#define _COMPOUND(diff_res) _MUL (diff_res, diff (node->left, context) )

#define _L         node->left
#define _R         node->right
#define _dL  diff (node->left, context)
#define _dR  diff (node->right, context)
#define _cL  copy (node->left)
#define _cR  copy (node->right)

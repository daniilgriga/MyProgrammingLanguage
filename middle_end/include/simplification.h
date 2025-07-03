#ifndef SIMPLIFICATION_H
#define SIMPLIFICATION_H

#include "tree.h"
#include "log.h"
#include "enum.h"

#define MAX_OPTIMIZATIONS 100

double eval (struct Node_t* node);

int simplification_typical_operations (struct Node_t* root, struct Node_t* parent);

void verificator (struct Node_t* node, const char* filename, int line);

int constant_folding (struct Node_t* root);

int simplification_of_expression (struct Node_t* root, struct Node_t* parent);

#endif // SIMPLIFICATION_H

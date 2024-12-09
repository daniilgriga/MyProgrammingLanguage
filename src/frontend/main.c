#include <stdio.h>

#include "tokens.h"

int main (void)
{
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = "sin(temp+5*size)/num^age$";

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    return 0;
}

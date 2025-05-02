#pragma once

struct Token_t;

struct Node_t* GetGrammar (struct Context_t* context);

[[noreturn]] void SyntaxError (struct Context_t* context, const char* filename, const char* func, int line, int error);

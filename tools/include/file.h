#pragma once

FILE* OpenFile (const char* filename, const char* mode);

long FileSize (FILE* file_ptr);

char* ReadInBuffer (FILE* file_ptr, const long numb_symb);

char* MakeBuffer (const char* filename, long* numb_symb);

enum Errors CloseFile (FILE* file_ptr);

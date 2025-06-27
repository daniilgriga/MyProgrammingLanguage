#pragma once

FILE* OpenFile (const char* filename, const char* mode);

enum Errors CloseFile (FILE* file_ptr);

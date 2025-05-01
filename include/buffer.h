#pragma once

char* file_reader (struct Buffer_t* buffer, const char* filename);

int count_symbols (struct Buffer_t* buffer, FILE* file);

#pragma once

FILE* open_log_file (const char* filename);

int log_printf (const char* message, ...);

void log_vprintf (const char* message, va_list args);

int write_log_file (const char* reason_bro, va_list args);

void close_log_file (void);

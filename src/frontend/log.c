#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#include "tree.h"
#include "log.h"

static FILE* LOG_FILE = NULL;

void log_vprintf (const char* message, va_list args)
{
    vfprintf (LOG_FILE, message, args);
}

int log_printf (const char* message, ...)
{
    va_list args;
    va_start (args, message);

    log_vprintf (message, args);

    va_end (args);

    return 0;
}

FILE* open_log_file (const char* filename)
{
    LOG_FILE = fopen (filename, "wb");
    if (LOG_FILE == NULL)
    {
        printf("ERROR");
        return NULL;
    }

    fprintf (LOG_FILE, "<pre>\n");

    return LOG_FILE;
}

int write_log_file (const char* reason, va_list args)
{
    log_printf ("<body style=\"background-color: #AFEEEE\">");

    log_printf ("<hr> <h2>");
    log_vprintf (reason, args);
    log_printf ("</h2> <br> <hr>\n\n");

    static int dump_number = 1;
    static char       way[50] = {};
    static char  filename[50] = {};
    char    command_name[200] = {};

    sprintf (way, "../../build/");
    sprintf (filename, "graph_tree%d.svg", dump_number++);
    sprintf (command_name, "dot ../../build/graph_tree.dot -Tsvg -o %s%s", way, filename);

    system  (command_name);

    log_printf ("\n\n<img src=\"%s\">", filename);

    fflush (LOG_FILE);

    return 0;
}

void close_log_file (void)
{
    fclose (LOG_FILE);
}

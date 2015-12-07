#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include "common.h"

void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stdout, "fail: ");
    vfprintf(stdout, fmt, ap);
    putchar('\n');
    exit(1);
    va_end(ap);
}


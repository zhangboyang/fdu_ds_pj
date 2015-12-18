#include <cstdio>
#include <cstdarg>
#include "common.h"
#include "printf2str.h"

std::string printf2str(const char *fmt, ...)
{
    static char buf[MAXLINE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return std::string(buf);
}


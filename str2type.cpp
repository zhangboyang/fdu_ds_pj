#include <cstdio>
#include "common.h"
#include "str2type.h"

LL str2LL(const char *s)
{
    assert(s);
    LL ret;
    int t = sscanf(s, LL_CS, &ret);
    if (t != 1) fail("can't convert '%s' to long long", s);
    return ret;
}

double str2double(const char *s)
{
    assert(s);
    double ret;
    int t = sscanf(s, "%lf", &ret);
    if (t != 1) fail("can't convert '%s' to double", s);
    return ret;
}


#include <ctime>
#include <cstdio>
#include "common.h"
#include "timing.h"

#ifdef DEBUG
static const char *last_comment;
#endif
static clock_t st, ed;

void timing_start(const char *comment)
{
#ifdef DEBUG
    assert(!last_comment);
    last_comment = comment;
#endif
    st = clock();
}

void timing_end(const char *comment)
{
    ed = clock();
#ifdef DEBUG
    last_comment = NULL;
#endif
    double t = (double) (ed - st) / CLOCKS_PER_SEC * 1000;
    printf("%s: %.2f ms\n", comment, t);
}

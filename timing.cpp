#include <ctime>
#include <cstdio>
#include "common.h"
#include "timing.h"

static const char *last_comment = NULL;
static clock_t st, ed;

void timing_start(const char *comment)
{
    assert(!last_comment);
    last_comment = comment;
    st = clock(); // clock is cpu-time in linux
}

void timing_end()
{
    ed = clock();
    double t = (double) (ed - st) / CLOCKS_PER_SEC * 1000;
    printf("%s: %.2f ms\n", last_comment, t);
    last_comment = NULL;
}

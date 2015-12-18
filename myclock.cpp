#include <sys/time.h>
#include <time.h>
#include "common.h"
#include "myclock.h"

static int init_flag = 0;
static struct timespec ts0;

static LL ts_minus(const struct timespec *tsp1, const struct timespec *tsp2)
{
    /* tsp1 - tsp2 (in us)
     * note: pay attation to rounding method */
    return (tsp1->tv_sec - tsp2->tv_sec) * 1000000 +
           (tsp1->tv_nsec - tsp2->tv_nsec) / 1000;
}

double myclock() // double should enough if you don't run 1e6 seconds (about 11 days)
{
    if (!init_flag) {
        init_flag = 1;
        clock_gettime(CLOCK_REALTIME, &ts0);
    }
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    LL us = ts_minus(&ts, &ts0);
    return us / 1000.0;
}

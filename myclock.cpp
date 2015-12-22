#ifdef ZBY_OS_LINUX
#include <sys/time.h>
#include <time.h>
#endif

#ifdef ZBY_OS_WINDOWS
#include <windows.h>
#endif

#include "common.h"
#include "myclock.h"

#ifdef ZBY_OS_LINUX
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
        clock_gettime(CLOCK_REALTIME, &ts0); // linux only
    }
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    LL us = ts_minus(&ts, &ts0);
    return us / 1000.0;
}
#endif


#ifdef ZBY_OS_WINDOWS
static int init_flag = 0;
static LARGE_INTEGER t0, freq;

double myclock()
{
    if (!init_flag) {
        init_flag = 1;
        QueryPerformanceFrequency(&freq); 
        QueryPerformanceCounter(&t0);
    }
    LARGE_INTEGER t1;
    QueryPerformanceCounter(&t1);
    LL t = t1.QuadPart - t0.QuadPart;
    return t * 1000.0 / freq.QuadPart;
}
#endif

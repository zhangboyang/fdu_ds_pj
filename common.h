#ifndef ZBY_COMMON_H
#define ZBY_COMMON_H

/* define NDEBUG if necessary (required by assert()) */
#ifndef DEBUG
#define NDEBUG
#endif

typedef long long LL;
typedef unsigned long long ULL;

/* assert() */
#include <cassert>

/* printd() ... */
#include "debug.h"

/* timing() ... */
#include "timing.h"

/* float and double function */
#include <cmath>
#define F_EPS 1e-10
#define F_INF 1e100
#define fequ(a, b) (fabs((a) - (b)) < F_EPS)
template <class T> inline static T sq(T a) { return a * a; }
inline static int fcmp(double x) { return fabs(x) < F_EPS ? 0 : (x > 0 ? 1 : -1); }

/* common buffer size */
#define MAXLINE 4096

/* long long conversion specifications with scanf and printf */
#ifdef ZBY_OS_WINDOWS
#define LL_CS "%I64d"
#else
#define LL_CS "%lld"
#endif

#endif

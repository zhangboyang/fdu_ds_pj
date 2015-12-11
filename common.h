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
#define eps 1e-6
#define fequ(a, b) (fabs((a) - (b)) < eps)
template <class T> inline static T sq(T a) { return a * a; }

/* common buffer size */
#define MAXLINE 4096

#endif

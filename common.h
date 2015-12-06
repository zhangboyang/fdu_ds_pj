#ifndef ZBY_COMMON_H
#define ZBY_COMMON_H

#define DEBUG

/* define NDEBUG if necessary (required by assert()) */
#ifndef DEBUG
#define NDEBUG
#endif

/* assert() */
#include <cassert>

/* printd() ... */
#include "debug.h"

/* timing() ... */
#include "timing.h"

#endif

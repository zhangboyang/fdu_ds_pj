#ifndef ZBY_DEBUG_H
#define ZBY_DEBUG_H
#include "common.h"

void fail(const char *fmt, ...);

/* printd */
#ifdef DEBUG
#define printd(fmt, ...) printf("[%s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
#define printd(fmt, ...) do { } while (0)
#endif

#endif

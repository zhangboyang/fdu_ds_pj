#ifndef ZBY_DEBUG_H
#define ZBY_DEBUG_H

void fail(const char *fmt, ...);
#define printd(fmt, ...) printf("[%s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#endif

#ifndef ZBY_TIMING_H
#define ZBY_TIMING_H
#include "common.h"

void timing_start(const char *comment);
void timing_end(const char *comment);

#define TIMING(STR, CODE) { timing_start(STR); { CODE } timing_end(STR); }

/*  TIMING usage:

    TIMING ("some comment", {
        some_code_here;
    })

    note: no semicolon at the end!
*/

#endif

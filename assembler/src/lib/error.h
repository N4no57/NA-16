#ifndef NA_16_ERROR_H
#define NA_16_ERROR_H

#include "asmlib.h"

extern i32 error_count;

void error(Position pos, const char *fmt, ...);
void fatal(Position pos, const char *fmt, ...);
void halt_on_error();

#endif //NA_16_ERROR_H

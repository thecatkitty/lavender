#ifndef NUMSTR_H
#define NUMSTR_H

#include <stdbool.h>
#include <stdint.h>

extern uint64_t
rstrtoull(const char *str, int base);

extern bool
isdigstr(const char *str);

extern bool
isxdigstr(const char *str);

extern uint8_t
xtob(const char *str);

#endif

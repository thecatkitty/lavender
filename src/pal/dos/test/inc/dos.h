#pragma once

#include <stdint.h>

extern unsigned
_dos_allocmem(unsigned __size, unsigned *__seg);

extern unsigned
_dos_freemem(unsigned __seg);

extern void *
MK_FP(unsigned __s, unsigned __o);

#ifndef _ARCH_ZIP_H_
#define _ARCH_ZIP_H_

#include <fmt/zip.h>

extern bool
ziparch_initialize(zip_archive self);

extern void
ziparch_cleanup(void);

#endif // _ARCH_ZIP_H_

#include <stdarg.h>
#include <stdio.h>

#include <pal.h>

void
pal_print_log(const char *location, const char *format, ...)
{
    va_list args;

    uint32_t counter = pal_get_counter();
    fprintf(stderr, "[%4u.%03u] ", counter / 1000, counter % 1000);
    fputs(location, stderr);
    fputs(": ", stderr);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);
}

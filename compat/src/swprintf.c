#include <stdarg.h>

#include <compat/wchar.h>

#ifndef swprintf_c
int
swprintf_c(wchar_t *buffer, size_t bufsz, const wchar_t *format, ...)
{
    int     result;
    va_list args;

    if (0 == bufsz)
    {
        return -1;
    }

    va_start(args, format);
    result = _vsnwprintf(buffer, bufsz - 1, format, args);
    va_end(args);

    buffer[bufsz - 1] = L'\0';
    return result;
}
#else
typedef int this_unit_is_intentionally_left_blank;
#endif

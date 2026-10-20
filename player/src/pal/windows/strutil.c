#include <wchar.h>

#include "impl.h"

void
windows_append(wchar_t *dst, const wchar_t *src, size_t size)
{
    wcsncat(dst, src, size - wcslen(dst));
}

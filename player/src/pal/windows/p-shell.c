#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

void
pal_open_url(const char *url)
{
    ShellExecuteA(NULL, NULL, url, NULL, NULL, SW_NORMAL);
}

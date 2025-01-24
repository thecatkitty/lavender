#include <arch/dos.h>
#include <arch/dos/winoldap.h>
#include <pal.h>

bool
pal_handle(void)
{
    if (!dos_is_windows())
    {
        return true;
    }

    if (0 == winoldap_query_close())
    {
        winoldap_acknowledge_close();
        return false;
    }

    return true;
}

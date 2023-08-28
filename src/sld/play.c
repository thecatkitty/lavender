#include <pal.h>
#include <snd.h>

#include "sld_impl.h"

int
__sld_execute_play(sld_entry *sld)
{
    if (!snd_play(sld->content))
    {
        __sld_errmsgcpy(sld, IDS_NOASSET);
        return SLD_SYSERR;
    }

    return 0;
}

#include <pal.h>
#include <snd.h>

#include "sld_impl.h"

int
__sld_execute_play(sld_entry *sld)
{
#if defined(CONFIG_SOUND)
    if (!snd_play(sld->content) && (ENOSYS != errno))
    {
        __sld_errmsgcpy(sld, IDS_NOASSET);
        return SLD_SYSERR;
    }
#endif // CONFIG_SOUND

    return 0;
}

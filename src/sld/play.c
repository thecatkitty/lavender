#include <string.h>

#include <pal.h>
#include <snd.h>

#include "sld_impl.h"

int
__sld_execute_play(sld_entry *sld)
{
    hasset music = pal_open_asset(sld->content, O_RDONLY);
    if (NULL == music)
    {
        strncpy((char *)sld, IDS_NOASSET, sizeof(sld_entry));
        return SLD_SYSERR;
    }

    char *data = pal_get_asset_data(music);
    if (NULL == data)
    {
        strncpy((char *)sld, IDS_NOASSET, sizeof(sld_entry));
        return SLD_SYSERR;
    }

    snd_play(data, pal_get_asset_size(music));
    return 0;
}

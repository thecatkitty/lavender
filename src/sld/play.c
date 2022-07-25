#include <pal.h>
#include <snd.h>

#include "sld_impl.h"

int
__sld_execute_play(sld_entry *sld)
{
    hasset music = pal_open_asset(sld->content, O_RDONLY);
    if (NULL == music)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(music);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    snd_play(data, pal_get_asset_size(music));
    return 0;
}

#include <stdio.h>

#include <fmt/zip.h>

#include <assets.h>

hasset
pal_open_asset(const char *name, int flags)
{
    off_t lfh;
    int   slot;

    LOG("entry, name: '%s', flags: %#x", name, flags);

    lfh = zip_search(name, strlen(name));
    if (-1 == lfh)
    {
        LOG("exit, cannot find asset. %s", strerror(errno));
        return NULL;
    }

    slot = 0;
    while (-1 != pal_assets[slot].inzip)
    {
        if (lfh == pal_assets[slot].inzip)
        {
            if (O_RDWR == (flags & O_ACCMODE))
            {
                pal_assets[slot].flags = (flags & ~O_ACCMODE) | O_RDWR;
            }

            LOG("exit, found a previously opened asset: %p",
                (void *)(pal_assets + slot));
            return (hasset)(pal_assets + slot);
        }

        slot++;
        if (MAX_OPEN_ASSETS == slot)
        {
            LOG("exit, no more slots");
            errno = ENOMEM;
            return NULL;
        }
    }

    LOG("exit, opened a new asset: %p", (void *)(pal_assets + slot));
    pal_assets[slot].inzip = lfh;
    pal_assets[slot].flags = flags;
    pal_assets[slot].opts = 0;
    pal_assets[slot].data = NULL;
    return (hasset)(pal_assets + slot);
}

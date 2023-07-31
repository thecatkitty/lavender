#include <string.h>

#include <fmt/zip.h>

#include "pal_impl.h"

pal_asset __pal_assets[MAX_OPEN_ASSETS];

hasset
pal_open_asset(const char *name, int flags)
{
    off_t lfh = zip_search(name, strlen(name));
    if (-1 == lfh)
    {
        return NULL;
    }

    int slot = 0;
    while (-1 != __pal_assets[slot].inzip)
    {
        if (lfh == __pal_assets[slot].inzip)
        {
            if (O_RDWR == (flags & O_ACCMODE))
            {
                __pal_assets[slot].flags = (flags & ~O_ACCMODE) | O_RDWR;
            }

            return (hasset)(__pal_assets + slot);
        }

        slot++;
        if (MAX_OPEN_ASSETS == slot)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    __pal_assets[slot].inzip = lfh;
    __pal_assets[slot].flags = flags;
    return (hasset)(__pal_assets + slot);
}

bool
pal_close_asset(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return false;
    }

    if (O_RDWR == (ptr->flags & O_ACCMODE))
    {
        // Do not release modified assets
        return true;
    }

    ptr->inzip = -1;

    if (NULL != ptr->data)
    {
        zip_free_data(ptr->data);
        ptr->data = NULL;
    }
    return true;
}

char *
pal_get_asset_data(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return NULL;
    }

    if (NULL == ptr->data)
    {
        ptr->data = zip_get_data(ptr->inzip);
    }

    return ptr->data;
}

int
pal_get_asset_size(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return -1;
    }

    return zip_get_size(ptr->inzip);
}

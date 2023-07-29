#include <string.h>

#include <fmt/zip.h>

#include "pal_impl.h"

pal_asset __pal_assets[MAX_OPEN_ASSETS];

hasset
pal_open_asset(const char *name, int flags)
{
    LOG("entry, name: '%s', flags: %#x", name, flags);

    off_t lfh = zip_search(name, strlen(name));
    if (-1 == lfh)
    {
        LOG("exit, cannot find asset. %s", strerror(errno));
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

            LOG("exit, found a previously opened asset: %p",
                __pal_assets + slot);
            return (hasset)(__pal_assets + slot);
        }

        slot++;
        if (MAX_OPEN_ASSETS == slot)
        {
            LOG("exit, no more slots");
            errno = ENOMEM;
            return NULL;
        }
    }

    LOG("exit, opened a new asset: %p", __pal_assets + slot);
    __pal_assets[slot].inzip = lfh;
    __pal_assets[slot].flags = flags;
    return (hasset)(__pal_assets + slot);
}

bool
pal_close_asset(hasset asset)
{
    LOG("entry, asset: %p", asset);

    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return false;
    }

    if (O_RDWR == (ptr->flags & O_ACCMODE))
    {
        // Do not release modified assets
        LOG("exit, not releasing");
        return true;
    }

    ptr->inzip = -1;

    if (NULL != ptr->data)
    {
        zip_free_data(ptr->data);
        ptr->data = NULL;
    }

    LOG("exit");
    return true;
}

char *
pal_get_asset_data(hasset asset)
{
    LOG("entry, asset: %p", asset);

    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return NULL;
    }

    if (NULL == ptr->data)
    {
        LOG("retrieving data for the first time");
        ptr->data = zip_get_data(ptr->inzip);
    }

    LOG("exit, %p", ptr->data);
    return ptr->data;
}

int
pal_get_asset_size(hasset asset)
{
    LOG("entry, asset: %p", asset);

    pal_asset *ptr = (pal_asset *)asset;
    if (-1 == ptr->inzip)
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return -1;
    }

    uint32_t size = zip_get_size(ptr->inzip);

    LOG("exit, %u", size);
    return size;
}

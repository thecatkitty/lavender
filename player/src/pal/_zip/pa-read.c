#include <fmt/zip.h>
#include <pal.h>

#include <assets.h>

bool
pal_read_asset(hasset asset, char *buff, off_t at, size_t size)
{
    pal_asset *ptr = (pal_asset *)asset;

    LOG("entry, asset: %p, %lu @ %lu", (void *)asset, (long)size, (long)at);

    if (-1 == ptr->inzip)
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return false;
    }

    if (O_RDWR == (ptr->flags & O_ACCMODE))
    {
        errno = EINVAL;
        LOG("exit, caching allowed for read-only");
        return false;
    }

    if (PALOPT_LOCAL == (ptr->opts & PALOPT_WHERE))
    {
        errno = EINVAL;
        LOG("exit, already opened in another mode");
        return false;
    }

    if (0 == ptr->handle)
    {
        LOG("retrieving data for the first time");
        ptr->handle = zip_cache(ptr->inzip);
    }

    if (0 >= (long)ptr->handle)
    {
        LOG("exit, failed with status %d", ptr->handle);
        ptr->handle = 0;
        return false;
    }

    ptr->opts |= PALOPT_CACHE;
    zip_read(ptr->handle, buff, at, size);

    return true;
}

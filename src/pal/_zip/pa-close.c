#include <fmt/zip.h>

#include <assets.h>

bool
pal_close_asset(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;

    LOG("entry, asset: %p", (void *)asset);

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

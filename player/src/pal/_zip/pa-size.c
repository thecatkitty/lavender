#include <fmt/zip.h>

#include <assets.h>

long
pal_get_asset_size(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;
    uint32_t   size;

    LOG("entry, asset: %p", (void *)asset);

    if ((NULL == asset) || (-1 == ptr->inzip))
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return -1;
    }

    size = zip_get_size(ptr->inzip);

    LOG("exit, %u", size);
    return size;
}

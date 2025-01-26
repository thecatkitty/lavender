#include <fmt/zip.h>

#include <assets.h>

char *
pal_get_asset_data(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;

    LOG("entry, asset: %p", (void *)asset);

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

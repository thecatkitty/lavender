#include <fmt/zip.h>

#include <assets.h>

char *
pal_load_asset(hasset asset)
{
    pal_asset *ptr = (pal_asset *)asset;

    LOG("entry, asset: %p", (void *)asset);

    if (-1 == ptr->inzip)
    {
        LOG("exit, wrong handle");
        errno = EBADF;
        return NULL;
    }

    if (PALOPT_CACHE == (ptr->opts & PALOPT_WHERE))
    {
        errno = EINVAL;
        LOG("exit, already opened in another mode");
        return false;
    }

    if (NULL == ptr->data)
    {
        LOG("retrieving data for the first time");
        ptr->data = zip_load_data(ptr->inzip);
    }

    LOG("exit, %p", ptr->data);
    return ptr->data;
}

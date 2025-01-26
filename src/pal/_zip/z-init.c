#include <arch/zip.h>
#include <fmt/zip.h>

#include <assets.h>

pal_asset pal_assets[MAX_OPEN_ASSETS];

bool
ziparch_initialize(zip_archive self)
{
    int i;

#ifdef ZIP_PIGGYBACK
    LOG("entry, piggyback");
#else
    LOG("entry, self: '%s'", self);
#endif

    for (i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        pal_assets[i].inzip = -1;
        pal_assets[i].flags = 0;
        pal_assets[i].data = NULL;
    }

    if (!zip_open(self))
    {
        LOG("cannot open the archive '%s'. %s", self, strerror(errno));
        return false;
    }

    return true;
}

void
ziparch_cleanup(void)
{
    int i;

    LOG("entry");

    for (i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        if (NULL != pal_assets[i].data)
        {
            zip_free_data(pal_assets[i].data);
        }
    }

    zip_close();
}

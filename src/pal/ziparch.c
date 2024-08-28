#ifdef __MINGW32__
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fmt/zip.h>

#include "../../ext/MatchingWildcards/Listing1.cpp"
#include "pal_impl.h"

pal_asset __pal_assets[MAX_OPEN_ASSETS];

#ifdef __ia16__
// newlib-ia16 seems to have some misbehaving tmpnam implementation
static int _tmpnam_num = 0;

static char *
_tmpnam(char *path)
{
    *path = 0;

    const char *tmp_dir = getenv("TMP");
    tmp_dir = tmp_dir ? tmp_dir : getenv("TEMP");
    if (tmp_dir)
    {
        strcpy(path, tmp_dir);
        strcat(path, "\\");
    }

    char name[13];
    sprintf(name, "~laven%02x.tmp", _tmpnam_num++);
    strcat(path, name);

    return path;
}
#elif defined(__MINGW32__)
#include <windows.h>

static char *
_tmpnam(char *path)
{
    if (0 == GetTempPathA(MAX_PATH, path))
    {
        return NULL;
    }

    if (0 == GetTempFileNameA(path, NULL, 0, path))
    {
        return NULL;
    }

    return path;
}
#else
#define _tmpnam tmpnam
#endif

#ifndef ZIP_PIGGYBACK
bool
ziparch_initialize(const char *self)
{
    LOG("entry, self: '%s'", self);

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        __pal_assets[i].inzip = -1;
        __pal_assets[i].flags = 0;
        __pal_assets[i].data = NULL;
    }

    if (!zip_open(self))
    {
        LOG("cannot open the archive '%s'. %s", self, strerror(errno));
        return false;
    }

    return true;
}
#endif

void
ziparch_cleanup(void)
{
    LOG("entry");

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        if (NULL != __pal_assets[i].data)
        {
            zip_free_data(__pal_assets[i].data);
        }
    }
}

typedef struct
{
    pal_enum_assets_callback callback;
    const char              *pattern;

    int   count;
    void *data;
} pal_enum_assets_ctx;

static bool
_zip_enum_files_callback(zip_cdir_file_header *cfh, void *data)
{
    pal_enum_assets_ctx *ctx = (pal_enum_assets_ctx *)data;

    char *name = alloca(cfh->name_length + 1);
    memcpy(name, cfh->name, cfh->name_length + 1);
    name[cfh->name_length] = 0;

    if (FastWildCompare((char *)ctx->pattern, name))
    {
        ctx->count++;
        return ctx->callback(name, ctx->data);
    }

    return true;
}

int
pal_enum_assets(pal_enum_assets_callback callback,
                const char              *pattern,
                void                    *data)
{
    pal_enum_assets_ctx ctx = {callback, pattern, 0, data};

    int status = zip_enum_files(_zip_enum_files_callback, &ctx);
    return (0 > status) ? status : ctx.count;
}

int
pal_extract_asset(const char *name, char *path)
{
    LOG("entry, name: '%s'", name);

    hasset asset = NULL;
    char  *data = NULL;
    FILE  *out = NULL;

    if (NULL == (asset = pal_open_asset(name, O_RDONLY)))
    {
        LOG("cannot open the asset!");
        goto exit;
    }

    if (NULL == (data = pal_get_asset_data(asset)))
    {
        LOG("cannot retrieve the asset data!");
        goto exit;
    }

    if (NULL == _tmpnam(path))
    {
        LOG("cannot create the temporary file name!");
        errno = ENOMEM;
        goto exit;
    }

    if (NULL == (out = fopen(path, "wb")))
    {
        LOG("cannot open the file!");
        goto exit;
    }

    int size = pal_get_asset_size(asset);
    if (size != fwrite(data, 1, size, out))
    {
        LOG("cannot write the file!");
        goto exit;
    }

    errno = 0;

exit:
    if (NULL != asset)
    {
        pal_close_asset(asset);
    }

    if (NULL != out)
    {
        fclose(out);
    }

    LOG("exit, %d", -errno);
    return -errno;
}

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
                (void *)(__pal_assets + slot));
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

    LOG("exit, opened a new asset: %p", (void *)(__pal_assets + slot));
    __pal_assets[slot].inzip = lfh;
    __pal_assets[slot].flags = flags;
    return (hasset)(__pal_assets + slot);
}

bool
pal_close_asset(hasset asset)
{
    LOG("entry, asset: %p", (void *)asset);

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
    LOG("entry, asset: %p", (void *)asset);

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
    LOG("entry, asset: %p", (void *)asset);

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

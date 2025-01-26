#include <stdlib.h>

#include <fmt/zip.h>
#include <pal.h>

#include <MatchingWildcards/Listing1.cpp>

typedef struct
{
    pal_enum_assets_callback callback;
    const char              *pattern;

    int   count;
    void *data;
} callback_context;

static bool
enum_callback(zip_cdir_file_header *cfh, void *data)
{
    callback_context *ctx = (callback_context *)data;

    char name[PATH_MAX] = {0};
    if (sizeof(name) < ((size_t)cfh->name_length + 1))
    {
        errno = EINVAL;
        return false;
    }

    strncpy(name, cfh->name, (size_t)cfh->name_length + 1);
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
    callback_context ctx = {callback, pattern, 0, data};

    int status = zip_enum_files(enum_callback, &ctx);
    return (0 > status) ? status : ctx.count;
}

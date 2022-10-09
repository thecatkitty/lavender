#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "sld_impl.h"

sld_context *
sld_create_context(const char *name, sld_context *parent)
{
    sld_context *ctx = (sld_context *)malloc(sizeof(sld_context));
    if (NULL == ctx)
    {
        errno = ENOMEM;
        return NULL;
    }

    ctx->script = pal_open_asset(name, O_RDWR);
    if (NULL == ctx->script)
    {
        free(ctx);
        return NULL;
    }

    ctx->data = pal_get_asset_data(ctx->script);
    if (NULL == ctx->data)
    {
        pal_close_asset(ctx->script);
        free(ctx);
        return NULL;
    }

    ctx->size = pal_get_asset_size(ctx->script);
    ctx->offset = 0;
    ctx->state = SLD_STATE_RUN;
    memset(ctx->message, 0, sizeof(ctx->message));
    ctx->parent = parent;
    return ctx;
}

bool
sld_close_context(sld_context *ctx)
{
    bool status = pal_close_asset(ctx->script);
    free(ctx);
    return status;
}

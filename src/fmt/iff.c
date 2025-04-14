#include <stdlib.h>

#include <fmt/iff.h>
#include <pal.h>

iff_context *
iff_open(hasset asset)
{
    iff_context *ctx;
    iff_head     head;

    ctx = (iff_context *)malloc(sizeof(iff_context));
    if (NULL == ctx)
    {
        return NULL;
    }

    ctx->asset = asset;
    ctx->position = 0;

    pal_read_asset(asset, (char *)&head, ctx->position, sizeof(head));
    ctx->type.dw = head.type.dw;

    if (IFF_FOURCC_RIFF.dw == head.type.dw)
    {
        ctx->mode = IFF_MODE_BE;
        ctx->length = head.length + sizeof(iff_head);
    }
    else if ((IFF_FOURCC_FORM.dw == head.type.dw) ||
             (IFF_FOURCC_LIST.dw == head.type.dw) ||
             (IFF_FOURCC_CAT.dw == head.type.dw))
    {
        ctx->mode = IFF_MODE_BE;
        ctx->length = head.length + sizeof(iff_head);
    }
    else
    {
        ctx->mode = IFF_MODE_BE | IFF_MODE_NOHEAD;
        ctx->length = pal_get_asset_size(asset);
    }

    return ctx;
}

bool
iff_close(iff_context *ctx)
{
    if (NULL == ctx)
    {
        errno = EINVAL;
        return false;
    }

    free(ctx);
    return true;
}

bool
iff_next_chunk(iff_context *ctx, iff_chunk *it)
{
    ptrdiff_t pos;

    if (0 == it->position)
    {
        it->position = ctx->position +
                       ((ctx->mode & IFF_MODE_NOHEAD) ? 0 : sizeof(iff_head));
        it->length = 0;
    }

    pos = it->position - ctx->position;
    if (0 > pos)
    {
        return false;
    }

    pos += it->length + (it->length & 1);
    if (ctx->length <= pos)
    {
        return false;
    }

    while (ctx->length > pos)
    {
        iff_head head;
        uint32_t length;

        pal_read_asset(ctx->asset, (char *)&head, ctx->position + pos,
                       sizeof(head));
        length = (ctx->mode & IFF_MODE_BE) ? BSWAP32(head.length) : head.length;

        if ((0 == it->type.dw) || (head.type.dw == it->type.dw))
        {
            it->type.dw = head.type.dw;
            it->length = length;
            it->position = ctx->position + pos + sizeof(iff_head);
            return true;
        }

        pos += sizeof(iff_head) + length + (length & 1);
    }

    return false;
}

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <fmt/bmp.h>
#include <pal.h>
#include <sld.h>

#include "sld_impl.h"

typedef struct
{
    gfx_bitmap bm;
    hasset     asset;
    int        x, y, height, last_y;
    float      scale;
    char _padding[SLD_ENTRY_MAX_LENGTH - ((sizeof(gfx_bitmap) + sizeof(hasset) +
                                           4 * sizeof(int) + sizeof(float)))];
    uint8_t state;
} bitmap_content;

#define CONTENT(sld) ((bitmap_content *)(&sld->content))
static_assert(sizeof(bitmap_content) <= sizeofm(sld_entry, content),
              "Bitmap context larger than available space");

enum
{
    STATE_START,
    STATE_READ,
    STATE_COMPLETE,
};

typedef struct
{
    size_t position;
    int    system_par;
    hasset asset;
    int    par;
} search_ctx;

static bool
_enum_assets_callback(const char *name, void *data)
{
    search_ctx *ctx = (search_ctx *)data;

    hasset asset;
    int    par;

    LOG("entry, name: '%s'", name);

    par = xtob(name + ctx->position);
    if (abs(ctx->par - ctx->system_par) <= abs(par - ctx->system_par))
    {
        LOG("exit, PAR %.2f worse than current %.2f", (float)par / 64.0f,
            (float)ctx->par / 64.0f);
        return true;
    }

    asset = pal_open_asset(name, O_RDONLY);
    if (NULL == asset)
    {
        LOG("exit, cannot open");
        return true;
    }

    if (NULL != ctx->asset)
    {
        pal_close_asset(ctx->asset);
    }

    LOG("exit, new best PAR %.2f", (float)par / 64.0f);
    ctx->asset = asset;
    ctx->par = par;
    return true;
}

static hasset
_find_best_bitmap(char *pattern)
{
    search_ctx ctx = {0};
    char      *placeholder;

    LOG("entry, pattern: '%s'", pattern);

    if (NULL == (placeholder = strstr(pattern, "<>")))
    {
        return pal_open_asset(pattern, O_RDONLY);
    }

    placeholder[0] = placeholder[1] = '?';
    ctx.position = placeholder - pattern;
    ctx.system_par = (int)gfx_get_pixel_aspect();
    ctx.par = INT_MAX;
    if (0 > pal_enum_assets(_enum_assets_callback, pattern, &ctx))
    {
        return NULL;
    }

    if (NULL == ctx.asset)
    {
        errno = ENOENT;
        return NULL;
    }

    return ctx.asset;
}

static int
execute_complete(sld_entry *sld);

static int
execute_start(sld_entry *sld)
{
#if defined(GFX_HAS_SCALE)
    float scale;
#endif
    gfx_bitmap     bm;
    gfx_dimensions screen;
    hasset         bitmap = _find_best_bitmap(sld->content);
    int            x, y;

    if (NULL == bitmap)
    {
        return SLD_SYSERR;
    }

    memset(&bm, 0, sizeof(bm));
    if (!bmp_load_bitmap(&bm, bitmap))
    {
        return SLD_SYSERR;
    }

    gfx_get_screen_dimensions(&screen);
#if defined(GFX_HAS_SCALE)
    scale = gfx_get_scale();
#endif

    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
#if defined(GFX_HAS_SCALE)
        x = ((float)screen.width / scale - bm.width) / 2 * scale;
#else
        x = (screen.width - bm.width) / 2;
#endif
        break;
    case SLD_ALIGN_RIGHT:
#if defined(GFX_HAS_SCALE)
        x = ((float)screen.width / scale - bm.width) * scale;
#else
        x = screen.width - bm.width;
#endif
        break;
    default:
        x = (int)((int32_t)sld->posx * screen.width / SLD_VIEWBOX_WIDTH);
    }

    y = (int)((int32_t)sld->posy * screen.height / SLD_VIEWBOX_HEIGHT);
    if (0 > bm.height)
    {
#if defined(GFX_HAS_SCALE)
        y -= (float)bm.height * scale;
#else
        y -= bm.height;
#endif
    }

    memcpy(&CONTENT(sld)->bm, &bm, sizeof(gfx_bitmap));
    CONTENT(sld)->asset = bitmap;
    CONTENT(sld)->x = x;
    CONTENT(sld)->y = y;
    CONTENT(sld)->height = 0;
    CONTENT(sld)->last_y = y;
#if defined(GFX_HAS_SCALE)
    CONTENT(sld)->scale = scale;
#endif
    CONTENT(sld)->state = STATE_READ;

    return CONTINUE;
}

static int
execute_read(sld_entry *sld)
{
    bitmap_content *ctx = CONTENT(sld);
    int             y = ctx->y;

    if (ctx->height >= abs(ctx->bm.height))
    {
        ctx->state = STATE_COMPLETE;
        return CONTINUE;
    }

    if (0 > ctx->bm.height)
    {
#if defined(GFX_HAS_SCALE)
        y = ctx->last_y - floor(ctx->bm.chunk_height * ctx->scale);
#else
        y -= ctx->bm.chunk_top + ctx->bm.chunk_height;
#endif
    }
    else
    {
#if defined(GFX_HAS_SCALE)
        y = ctx->last_y + floor(ctx->bm.chunk_height * ctx->scale);
#else
        y += ctx->bm.chunk_top;
#endif
    }

    if (!gfx_draw_bitmap(&ctx->bm, ctx->x, y))
    {
        execute_complete(sld);
        return SLD_SYSERR;
    }

    ctx->height += ctx->bm.chunk_height;
    ctx->last_y = y;
    if ((ctx->height < abs(ctx->bm.height)) &&
        !bmp_load_bitmap(&ctx->bm, ctx->asset))
    {
        execute_complete(sld);
        return SLD_SYSERR;
    }

    return CONTINUE;
}

static int
execute_complete(sld_entry *sld)
{
    bmp_dispose_bitmap(&CONTENT(sld)->bm);
    pal_close_asset(CONTENT(sld)->asset);
    return 0;
}

int
__sld_execute_bitmap(sld_entry *sld)
{
    switch (CONTENT(sld)->state)
    {
    case STATE_START:
        return execute_start(sld);

    case STATE_READ:
        return execute_read(sld);

    case STATE_COMPLETE:
        return execute_complete(sld);
    }

    assert(false && "wrong state");
    errno = -ENOSYS;
    return SLD_SYSERR;
}

int
__sld_load_bitmap(const char *str, sld_entry *out)
{
    const char *cur = str;

    __sld_try_load(__sld_load_position, cur, out);
    __sld_try_load(__sld_load_content, cur, out);

    CONTENT(out)->state = STATE_START;

    return cur - str;
}

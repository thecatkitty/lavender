#include <stdlib.h>

#include <fmt/bmp.h>
#include <pal.h>
#include <sld.h>

#include "sld_impl.h"

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

int
__sld_execute_bitmap(sld_entry *sld)
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

    if (!gfx_draw_bitmap(&bm, x, y))
    {
        return SLD_SYSERR;
    }

    pal_close_asset(bitmap);
    return 0;
}

int
__sld_load_bitmap(const char *str, sld_entry *out)
{
    const char *cur = str;

    __sld_try_load(__sld_load_position, cur, out);
    __sld_try_load(__sld_load_content, cur, out);

    return cur - str;
}

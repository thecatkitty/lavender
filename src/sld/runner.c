#include <string.h>

#include <api/bios.h>
#include <pal.h>

#include "sld_impl.h"

uint16_t       __sld_accumulator = 0;
gfx_dimensions __sld_screen;

// Execute a line loaded from the script
// Returns negative on error
static int
_execute_entry(sld_entry *sld)
{
    if (0 == __sld_screen.width)
    {
        gfx_get_screen_dimensions(&__sld_screen);
    }

    pal_sleep(sld->delay);

    switch (sld->type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return __sld_execute_text(sld);
    case SLD_TYPE_BITMAP:
        return __sld_execute_bitmap(sld);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return __sld_execute_rectangle(sld);
    case SLD_TYPE_PLAY:
        return __sld_execute_play(sld);
    case SLD_TYPE_WAITKEY:
        __sld_accumulator = bios_get_keystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (__sld_accumulator == sld->posy) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return __sld_execute_script_call(sld);
    }

    strncpy((char *)sld, IDS_UNKNOWNTYPE, sizeof(sld_entry));
    return SLD_ARGERR;
}

// Find the first line after the given label
// Returns negative on error
static int
_goto_label(sld_context *ctx, const char *label)
{
    sld_entry entry;
    int       length;

    ctx->offset = 0;
    while (ctx->offset < ctx->size)
    {
        length = sld_load_entry(ctx, &entry);
        if (0 > length)
        {
            return length;
        }

        ctx->offset += length;

        if (SLD_TYPE_LABEL != entry.type)
            continue;

        if (0 == strcmp(label, entry.content))
        {
            return 0;
        }
    }

    char msg[sizeof(sld_entry)];
    strncpy(msg, IDS_NOLABEL, sizeof(sld_entry));
    strcat(msg, ": ");
    strncat(msg, label, sizeof(sld_entry) - strlen(msg));
    strcpy(ctx->message, msg);
    return SLD_ARGERR;
}

int
sld_run_script(sld_context *ctx)
{
    int length;

    while (ctx->offset < ctx->size)
    {
        if (0 > (length = sld_load_entry(ctx, &ctx->entry)))
        {
            char msg[sizeof(sld_entry)];
            strncpy(msg, IDS_LOADERROR, sizeof(sld_entry));
            strcat(msg, "\n");
            strncat(msg, ctx->message, sizeof(sld_entry) - strlen(ctx->message));
            strcpy(ctx->message, msg);
            return length;
        }

        int status;
        if (0 > (status = _execute_entry(&ctx->entry)))
        {
            char msg[sizeof(sld_entry)];
            strncpy(msg, IDS_EXECERROR, sizeof(sld_entry));
            strcat(msg, "\n");
            strncat(msg, ctx->message, sizeof(sld_entry) - strlen(ctx->message));
            strcpy(ctx->message, msg);
            return status;
        }

        if (INT_MAX == status)
        {
            if (0 > (length = _goto_label(ctx, ctx->entry.content)))
            {
                return length;
            }
            continue;
        }

        ctx->offset += length;
    }

    return 0;
}

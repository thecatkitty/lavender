#include <pal.h>
#include <snd.h>

#include "sld_impl.h"

uint16_t       __sld_accumulator = 0;
gfx_dimensions __sld_screen;

// Execute a line loaded from the script
// Returns CONTINUE on continuation request, negative on error
static int
_execute_entry(sld_entry *sld)
{
    if (0 == __sld_screen.width)
    {
        gfx_get_screen_dimensions(&__sld_screen);
    }

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
#if PAL_EXTERNAL_TICK
        pal_stall(-1);
#endif
        pal_enable_mouse();
        __sld_ctx->state = SLD_STATE_WAIT;
        return 0;
    case SLD_TYPE_ACTAREA:
        return __sld_execute_active_area(sld);
    case SLD_TYPE_QUERY:
        return __sld_execute_query(sld);
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        if (__sld_accumulator == sld->posy)
        {
            if (SLD_ACTAREA_HOVER > __sld_accumulator)
            {
                pal_disable_mouse();
            }
            return INT_MAX;
        }

        return 0;
    case SLD_TYPE_CALL:
        return __sld_handle_script_call(sld);
    }

    __sld_errmsgcpy(sld, IDS_UNKNOWNTYPE);
    return SLD_ARGERR;
}

// Find the first line after the given label
// Returns negative on error
static int
_goto_label(sld_context *ctx, const char *label)
{
    sld_entry entry;
    int       length;
    char      msg[sizeof(sld_entry)] = {0};

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

    __sld_errmsgcpy(msg, IDS_NOLABEL);
    __sld_errmsgcat(msg, ": ");
    __sld_errmsgcat(msg, label);
    strcpy(ctx->message, msg);
    return SLD_ARGERR;
}

void
sld_handle(void)
{
    sld_context *ctx = __sld_ctx;
    int          status;

    if ((SLD_QUIT != ctx->state) && !pal_handle())
    {
        __sld_ctx->state = SLD_QUIT;
        return;
    }

#if defined(CONFIG_SOUND) && !PAL_EXTERNAL_TICK
    snd_handle();
#endif

    if (NULL == __sld_ctx)
    {
        return;
    }

    // SLD_STATE_STOP, SLD_QUIT
    if ((SLD_STATE_STOP == ctx->state) || (SLD_QUIT == ctx->state))
    {
        sld_context *old_ctx = sld_exit_context();
        if (0 > old_ctx->state)
        {
            __sld_ctx->state = old_ctx->state;
        }
        sld_close_context(old_ctx);
        return;
    }

    // SLD_STATE_WAIT
    if (SLD_STATE_WAIT == ctx->state)
    {
        uint16_t x, y, buttons, tag;

        int keystroke = pal_get_keystroke();
        if (0 != keystroke)
        {
            __sld_accumulator = keystroke;
            __sld_ctx->state = SLD_STATE_LOAD;
            return;
        }

        buttons = pal_get_mouse(&x, &y);
        tag = __sld_retrieve_active_area_tag(x, y);
        if (0 != tag)
        {
            __sld_ctx->state = SLD_STATE_LOAD;

            if (buttons & PAL_MOUSE_LBUTTON)
            {
                __sld_accumulator = tag;
            }
            else
            {
                __sld_accumulator = SLD_ACTAREA_HOVER + tag;
            }
        }
        else if (SLD_ACTAREA_HOVER < __sld_accumulator)
        {
            __sld_ctx->state = SLD_STATE_LOAD;
            __sld_accumulator = SLD_ACTAREA_HOVER;
        }

        return;
    }

    // SLD_STATE_LOAD
    if (SLD_STATE_LOAD == ctx->state)
    {
        int length;

        if (ctx->offset >= ctx->size)
        {
            ctx->state = SLD_STATE_STOP;
            return;
        }

        length = sld_load_entry(ctx, &ctx->entry);
        if (0 > length)
        {
            char msg[sizeof(sld_entry)] = {0};
            __sld_errmsgcpy(msg, IDS_LOADERROR);
            __sld_errmsgcat(msg, "\n");
            __sld_errmsgcat(msg, ctx->message);
            strcpy(ctx->message, msg);
            ctx->state = length;
            return;
        }

        ctx->offset += length;

#if PAL_EXTERNAL_TICK
        pal_stall(ctx->entry.delay / 4);
#endif
        if (0 != ctx->entry.delay)
        {
            ctx->next_step =
                pal_get_counter() + pal_get_ticks(ctx->entry.delay);
            ctx->state = SLD_STATE_DELAY;
        }
        else
        {
            ctx->state = SLD_STATE_EXECUTE;
        }
        return;
    }

    // SLD_STATE_DELAY
    if (SLD_STATE_DELAY == ctx->state)
    {
        if (ctx->next_step < pal_get_counter())
        {
            ctx->state = SLD_STATE_EXECUTE;
        }
        return;
    }

    // SLD_STATE_EXECUTE
    status = _execute_entry(&ctx->entry);
    if (CONTINUE == status)
    {
        return;
    }

    if (0 > status)
    {
        char msg[sizeof(sld_entry)] = {0};
        __sld_errmsgcpy(msg, IDS_EXECERROR);
        __sld_errmsgcat(msg, "\n");
        __sld_errmsgcat(msg, ctx->message);
        strcpy(ctx->message, msg);
        ctx->state = status;
        return;
    }

    if (SLD_STATE_EXECUTE == ctx->state)
    {
        ctx->state = SLD_STATE_LOAD;
    }

    // Special value for jumps
    if (INT_MAX == status)
    {
        _goto_label(ctx, ctx->entry.content);
        return;
    }
}

void
sld_run(sld_context *ctx)
{
    ctx->state = SLD_STATE_LOAD;
}

#include <string.h>

#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

#include "../../../resource.h"
#include "direct.h"

enum
{
    STATE_NONE,
    STATE_PROMPT,
    STATE_VERIFY,
};

static int         _state = STATE_NONE;
static encui_page *_pages = NULL;
static int         _id;

bool
encui_enter(encui_page *pages, int count)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    encui_direct_init_frame();
    _pages = pages;
    _id = -1;
    return true;
}

bool
encui_exit(void)
{
    _state = STATE_NONE;
    return true;
}

static void
_reset(void)
{
    pal_disable_mouse();
    _state = STATE_NONE;
}

int
encui_handle(void)
{
    encui_page        *page = _pages + _id;
    encui_prompt_page *prompt = encui_find_prompt(_pages + _id);

    if (STATE_NONE == _state)
    {
        return 0;
    }

    if (!encui_direct_animate(true))
    {
        return ENCUI_INCOMPLETE;
    }

    if (STATE_VERIFY == _state)
    {
        int status = page->proc(ENCUIM_NEXT, prompt->buffer, page->data);
        if ((0 == status) || (-ENOSYS == status))
        {
            _reset();
            return prompt->length;
        }

        if (0 > status)
        {
            return status;
        }

        char message[GFX_COLUMNS];
        if (INT_MAX == status)
        {
            strncpy(message, prompt->alert, GFX_COLUMNS - 1);
            free((void *)prompt->alert);
            prompt->alert = NULL;
        }
        else
        {
            pal_load_string(status, message, sizeof(message));
        }
        encui_direct_set_error(message);

        _state = STATE_PROMPT;
        pal_enable_mouse();
        return ENCUI_INCOMPLETE;
    }

    int      status = 0;
    uint16_t x, y;
    if (PAL_MOUSE_LBUTTON & pal_get_mouse(&x, &y))
    {
        status = encui_direct_click(x, y);
    }
    else
    {
        status = encui_direct_key(pal_get_keystroke());
    }

    if (ENCUI_OK == status)
    {
        _state = STATE_VERIFY;
    }
    else if (ENCUI_CANCEL == status)
    {
        _reset();
        return 0;
    }

    return ENCUI_INCOMPLETE;
}

int
encui_get_page(void)
{
    return _id;
}

bool
encui_set_page(int id)
{
    if (_id == id)
    {
        return true;
    }

    encui_page *page = _pages + id;
    _id = id;

    if (0 == page->title)
    {
        _id = -1;
        _state = STATE_NONE;
        return true;
    }

    encui_direct_enter_page(_pages, id);
    _state = STATE_PROMPT;
    return true;
}

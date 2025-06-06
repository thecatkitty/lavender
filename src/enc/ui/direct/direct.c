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

static bool _notify = false;
static int  _cookie;

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
    encui_page         *page = _pages + _id;
    encui_textbox_data *textbox = encui_find_textbox(_pages + _id);

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
        int status = page->proc(ENCUIM_NEXT, textbox ? textbox->buffer : NULL,
                                page->data);
        if ((0 == status) || (-ENOSYS == status))
        {
            _reset();
            return textbox ? textbox->length : 1;
        }

        if (-EINTR == status)
        {
            // Navigation interrupted
            _state = STATE_PROMPT;
            pal_enable_mouse();
            return ENCUI_INCOMPLETE;
        }

        if (0 > status)
        {
            return status;
        }

        char message[GFX_COLUMNS];
        if ((INT_MAX == status) && (NULL != textbox))
        {
            strncpy(message, textbox->alert, GFX_COLUMNS - 1);
            free((void *)textbox->alert);
            textbox->alert = NULL;
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

    if (_notify)
    {
        _notify = false;
        page->proc(ENCUIM_NOTIFY, (void *)(intptr_t)_cookie, page->data);
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

    if ((0 == page->title) && (NULL == page->data))
    {
        _id = -1;
        _state = STATE_NONE;
        return true;
    }

    if (0 == page->title)
    {
        id = (intptr_t)page->data;
    }

    _id = id;
    encui_direct_enter_page(_pages, id);
    _state = STATE_PROMPT;
    return true;
}

int
encui_check_page(const encui_page *page, void *param)
{
    return page->proc(ENCUIM_CHECK, param, page->data);
}

bool
encui_request_notify(int cookie)
{
    if (_notify)
    {
        return false;
    }

    _notify = true;
    _cookie = cookie;
    return true;
}

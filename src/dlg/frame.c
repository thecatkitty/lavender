#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <dlg.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

gfx_dimensions screen = {640, 200};

int
DlgDrawBackground(void)
{
    gfx_fill_rectangle(&screen, 0, 0, GFX_COLOR_GRAY);

    gfx_dimensions bar = {screen.width, 9};
    gfx_fill_rectangle(&bar, 0, 0, GFX_COLOR_WHITE);

    gfx_dimensions hline = {screen.width - 1, 1};
    gfx_draw_line(&hline, 0, bar.height, GFX_COLOR_BLACK);

    const char *brand = pal_get_version_string();
    for (int i = 0; brand[i]; i++)
    {
        bios_set_cursor_position(0, i + 1);
        bios_write_character(0, brand[i], 0x80, 1);
    }

    gfx_fill_rectangle(&bar, 0, screen.height - bar.height, GFX_COLOR_BLACK);
    gfx_draw_text("(C) 2021-2022", 1, 24);
    gfx_draw_text("https://github.com/thecatkitty/lavender/", 39, 24);

    return 0;
}

int
DlgDrawFrame(DLG_FRAME *frame, const char *title)
{
    gfx_dimensions window = {8 * (frame->Columns + 3), 8 * (frame->Lines + 3)};

    int left = (screen.width - window.width) / 2;
    int top = (screen.height - window.height) / 16 * 8;
    gfx_fill_rectangle(&window, left, top, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&window, left, top, GFX_COLOR_BLACK);
    window.width += 2;
    gfx_draw_rectangle(&window, left - 1, top, GFX_COLOR_BLACK);

    gfx_dimensions titleLine = {window.width - 1, 1};
    gfx_draw_line(&titleLine, left, top + 9, GFX_COLOR_BLACK);

    char *titleConv = alloca(strlen(title) + 1);
    utf8_encode(title, titleConv, gfx_wctob);

    int            titleLength = strlen(titleConv);
    gfx_dimensions stripe = {(window.width - ((titleLength + 2) * 8)) / 2 - 1,
                             1};
    for (int i = 0; i < 4; i++)
    {
        int y = top + 1 + i * 2;
        gfx_draw_line(&stripe, left + 1, y, GFX_COLOR_BLACK);
        gfx_draw_line(&stripe, left + window.width - stripe.width - 4, y,
                      GFX_COLOR_BLACK);
    }

    gfx_draw_text(titleConv, (80 - titleLength) / 2, top / 8);
    return 0;
}

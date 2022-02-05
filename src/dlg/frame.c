#include <string.h>

#include <api/bios.h>
#include <dlg.h>
#include <gfx.h>
#include <vid.h>

int
DlgShowFrame(DLG_FRAME *frame, const char *title)
{
    const GFX_DIMENSIONS screen = {640, 200};

    GFX_DIMENSIONS bar = {screen.Width, 9};
    GFX_DIMENSIONS hline = {screen.Width, 1};
    GFX_DIMENSIONS bg = {screen.Width, screen.Height - 2 * bar.Height};
    GFX_DIMENSIONS window = {8 * (frame->Columns + 3), 8 * (frame->Lines + 3)};

    VidFillRectangle(&bar, 0, 0, GFX_COLOR_WHITE);
    VidDrawLine(&hline, 0, bar.Height, GFX_COLOR_BLACK);

    const char *brand = (const char *)0x100;
    while ('L' != *brand)
    {
        brand++;
    }

    for (int i = 0; 0x1A != brand[i]; i++)
    {
        BiosVideoSetCursorPosition(0, i + 1);
        BiosVideoWriteCharacter(0, brand[i], 0x80, 1);
    }

    VidFillRectangle(&bg, 0, bar.Height + 1, GFX_COLOR_GRAY50);

    VidDrawText("(C) 2021-2022", 1, 24);
    VidDrawText("https://github.com/thecatkitty/lavender/", 39, 24);

    int left = (screen.Width - window.Width) / 2;
    int top = (screen.Height - window.Height) / 16 * 8;
    VidFillRectangle(&window, left, top, GFX_COLOR_WHITE);
    VidDrawRectangle(&window, left, top, GFX_COLOR_BLACK);
    window.Width += 2;
    VidDrawRectangle(&window, left - 1, top, GFX_COLOR_BLACK);

    GFX_DIMENSIONS titleLine = {window.Width - 1, 1};
    VidDrawLine(&titleLine, left, top + 9, GFX_COLOR_BLACK);

    int            titleLength = strlen(title);
    GFX_DIMENSIONS stripe = {(window.Width - ((titleLength + 2) * 8)) / 2 - 1,
                             1};
    for (int i = 0; i < 4; i++)
    {
        int y = top + 1 + i * 2;
        VidDrawLine(&stripe, left + 1, y, GFX_COLOR_BLACK);
        VidDrawLine(&stripe, left + window.Width - stripe.Width - 4, y,
                    GFX_COLOR_BLACK);
    }

    VidDrawText(title, (80 - titleLength) / 2, top / 8);
    return 0;
}

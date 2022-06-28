#include <api/bios.h>
#include <dlg.h>
#include <gfx.h>
#include <pal.h>
#include <vid.h>

int
DlgInputText(DLG_FRAME *frame,
             char      *buffer,
             int        size,
             bool (*validate)(const char *),
             int line)
{
    int fieldWidth = (frame->Columns > size) ? size : frame->Columns;
    int fieldLeft = (80 - fieldWidth) / 2;
    int fieldTop = (22 - frame->Lines) / 2 + 2 + line;

    GFX_DIMENSIONS box = {fieldWidth * 8 + 2, 10};
    int            boxTop = fieldTop * 8 - 1;
    int            boxLeft = fieldLeft * 8 - 1;

    VidDrawRectangle(&box, boxLeft, boxTop, GFX_COLOR_BLACK);
    buffer[0] = 0;

    int cursor = 0;
    while (true)
    {

        if (validate)
        {
            VidDrawRectangle(&box, boxLeft, boxTop,
                             validate(buffer) ? GFX_COLOR_BLACK
                                              : GFX_COLOR_GRAY50);
        }

        VidFillRectangle(&box, boxLeft, boxTop, GFX_COLOR_WHITE);
        VidDrawText(buffer, fieldLeft, fieldTop);

        uint16_t key = bios_get_keystroke();
        uint8_t  scanCode = key >> 8, character = key & 0xFF;
        if (0x01 == scanCode)
        {
            return 0;
        }

        if (0x1C == scanCode)
        {
            if (validate && validate(buffer))
            {
                return cursor;
            }

            if (!validate)
            {
                return cursor;
            }

            VidDrawRectangle(&box, boxLeft, boxTop, GFX_COLOR_BLACK);
            pal_sleep(63);
            VidDrawRectangle(&box, boxLeft, boxTop, GFX_COLOR_GRAY50);
            pal_sleep(63);
            VidDrawRectangle(&box, boxLeft, boxTop, GFX_COLOR_BLACK);
            pal_sleep(63);
            continue;
        }

        if ((0x0E == scanCode) && (0 < cursor))
        {
            cursor--;
            buffer[cursor] = 0;
        }

        if ((0x20 <= character) && (0x80 > character) && (cursor < size))
        {
            buffer[cursor] = key & 0xFF;
            cursor++;
            buffer[cursor] = 0;
        }
    }
}

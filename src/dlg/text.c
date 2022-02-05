#include <string.h>

#include <dlg.h>
#include <vid.h>

int
DlgDrawText(DLG_FRAME *frame, const char *str, int line)
{
    return VidDrawText(str, (80 - frame->Columns) / 2,
                       (25 - frame->Lines) / 2 + line);
}

#include <stdlib.h>
#include <string.h>

#include <dlg.h>
#include <fmt/utf8.h>
#include <vid.h>

int
DlgDrawText(DLG_FRAME *frame, const char *str, int line)
{
    char *strConv = alloca(strlen(str) + 1);
    utf8_encode(str, strConv, VidConvertToLocal);
    return VidDrawText(strConv, (80 - frame->Columns) / 2,
                       (22 - frame->Lines) / 2 + line + 2);
}

#include <stdlib.h>
#include <string.h>

#include <cvt.h>
#include <dlg.h>
#include <vid.h>

int
DlgDrawText(DLG_FRAME *frame, const char *str, int line)
{
    char *strConv = alloca(strlen(str) + 1);
    cvt_utf8_encode(str, strConv, VidConvertToLocal);
    return VidDrawText(strConv, (80 - frame->Columns) / 2,
                       (22 - frame->Lines) / 2 + line + 2);
}

#include <stdlib.h>
#include <string.h>

#include <dlg.h>
#include <ker.h>
#include <vid.h>

int
DlgDrawText(DLG_FRAME *frame, const char *str, int line)
{
    char *strConv = alloca(strlen(str) + 1);
    KerConvertFromUtf8(str, strConv, VidConvertToLocal);
    return VidDrawText(strConv, (80 - frame->Columns) / 2,
                       (22 - frame->Lines) / 2 + line + 2);
}

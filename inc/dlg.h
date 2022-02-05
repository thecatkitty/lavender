#ifndef _DLG_H_
#define _DLG_H_

#ifndef __ASSEMBLER__

#include <base.h>

typedef struct
{
    int Columns;
    int Lines;
} DLG_FRAME;

extern int
DlgDrawBackground(void);

extern int
DlgDrawFrame(DLG_FRAME *frame, const char *title);

extern int
DlgDrawText(DLG_FRAME *frame, const char *str, int line);

extern int
DlgInputText(DLG_FRAME *frame,
             char *     buffer,
             int        size,
             bool (*validate)(const char *),
             int line);

#endif // __ASSEMBLER__

#endif // _DLG_H_

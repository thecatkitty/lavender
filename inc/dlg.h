#ifndef _DLG_H_
#define _DLG_H_

#ifndef __ASSEMBLER__

typedef struct
{
    int Columns;
    int Lines;
} DLG_FRAME;

extern int
DlgShowFrame(DLG_FRAME *frame, const char *title);

extern int
DlgDrawText(DLG_FRAME *frame, const char *str, int line);

#endif // __ASSEMBLER__

#endif // _DLG_H_

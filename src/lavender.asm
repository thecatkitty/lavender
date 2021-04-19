%include "vid.inc"
%include "line.inc"


                cpu     8086

                extern  _ext_start

[bits 16]
section .init


                jmp     LavenderEntry


section .text


LavenderEntry:
                mov     ax, VID_MODE_CGA_HIMONO
                call    VidSetMode
                push    ax              ; save previous mode on stack

                mov     si, oBitmap
                mov     ah, (640 - LOGOW) / 2 / 8
                mov     al, (144 - LOGOH) / 2
                mov     cx, LOGOW
                mov     dx, LOGOH
                call    VidDrawBitmap

                mov     si, _ext_start
.Next:
                mov     di, oLine
                call    LineLoad
                jc      .Error
                test    ax, ax
                jz      .End
                push    si

                call    LineExecute
                pop     si
                jmp    .Next

.End:
                xor     ah, ah          ; GET KEYSTROKE
                int     16h

                pop     ax              ; restore saved mode
                call    VidSetMode

                mov     ax, 4C00h       ; TERMINATE WITH RETURN CODE -> 0
                int     21h
.Error:
                push    ax
                mov     ah, 09h         ; WRITE STRING TO STANDARD OUTPUT
                mov     dx, sErr
                int     21h

                pop     ax
                mov     ah, 4Ch         ; TERMINATE WITH RETURN CODE -> AL
                int     21h


section .data


LOGOW                           equ     272             ; logo width
LOGOH                           equ     100             ; logo height

oBitmap                         incbin  "../data/cgihisym.pbm", 57
sErr                            db      "ERR$"


section .bss


oLine                           resb    LINE_size

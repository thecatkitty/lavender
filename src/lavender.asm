%include "bios.inc"
%include "dos.inc"
%include "line.inc"
%include "vid.inc"


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
                xor     ah, ah          ; AH = BIOS_KEYBOARD_GET_KEYSTROKE
                int     BIOS_INT_KEYBOARD

                pop     ax              ; restore saved mode
                call    VidSetMode

                mov     ax, (DOS_EXIT << 8 | 0)
                int     DOS_INT
.Error:
                push    ax
                mov     ah, DOS_PUTS
                mov     dx, sErr
                int     DOS_INT

                pop     ax
                mov     ah, DOS_EXIT
                int     DOS_INT


section .data


LOGOW                           equ     272             ; logo width
LOGOH                           equ     100             ; logo height

oBitmap                         incbin  "../data/cgihisym.pbm", 57
sErr                            db      "ERR$"


section .bss


oLine                           resb    LINE_size

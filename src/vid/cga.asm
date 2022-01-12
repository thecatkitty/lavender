%define VID_API
%include "err.inc"
%include "gfx.inc"
%include "ker.inc"
%include "vid.inc"
%include "api/bios.inc"
%include "api/dos.inc"
%include "dev/cga.inc"


                cpu     8086

[bits 16]
section .text


                global  VidDrawText
VidDrawText:
                push    dx              ; save registers
                push    bx
                push    ax

                mov     dh, al
                mov     dl, ah
                xor     bx, bx
                mov     ah, BIOS_VIDEO_SET_CURSOR_POSITION
                int     BIOS_INT_VIDEO

                mov     ah, DOS_PUTC
.Next:
                mov     dl, [si]
                test    dl, dl
                jz      .End
                int     DOS_INT
                inc     si
                jmp     .Next

.End:           pop     ax
                pop     bx
                pop     cx
                ret

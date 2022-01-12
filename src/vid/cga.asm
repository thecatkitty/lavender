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


                global  VidDrawBitmap
VidDrawBitmap:
                cmp     byte [si + GFX_BITMAP.bPlanes], 1
                jne     .BadFormat
                mov     byte [si + GFX_BITMAP.bBitsPerPixel], 1
                jne     .BadFormat

                push    ax
                push    cx
                push    dx
                push    si
                push    di
                push    es

                mov     di, ax          ; DI = x
                mov     ax, VID_CGA_HIMONO_LINE / 2
                mul     bx
                add     di, ax          ; DX:AX = y * VID_CGA_HIMONO_LINE / 2 + x

                mov     cx, word [si + GFX_BITMAP.wWidthBytes]
                mov     dx, word [si + GFX_BITMAP.wHeight]
                mov     si, word [si + GFX_BITMAP.pBits]

                mov     ax, CGA_HIMONO_MEM     
                mov     es, ax
.Next:
                call    CgaDrawBitmapLine
                xor     di, CGA_HIMONO_PLANE
                dec     dx              ; even lines

                call    CgaDrawBitmapLine
                add     di, VID_CGA_HIMONO_LINE
                xor     di, CGA_HIMONO_PLANE
                dec     dx              ; odd lines
                jnz     .Next
.Error:
.End:           pop     es
                pop     di
                pop     si
                pop     dx
                pop     cx
                pop     ax
                ret
.BadFormat:     ERR     VID_FORMAT


; Copy bitmap line to screen buffer
; Input:
;   DS:SI - bitmap line
;   ES:DI - screen line
;   CX    - number of octets
; Output:
;   DS:SI - next bitmap line
CgaDrawBitmapLine:
                push    di
                push    cx
                cld
                rep     movsb
                pop     cx
                pop     di
                ret


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

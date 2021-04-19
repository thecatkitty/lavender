%define VID_API
%include "bios.inc"
%include "dos.inc"
%include "vid.inc"


                cpu     8086

[bits 16]
section .text


                global  VidSetMode
VidSetMode:
                push    ax
                mov     ah, BIOS_VIDEO_GET_MODE
                int     BIOS_INT_VIDEO
                xor     ah, ah          ; store previous video mode in CX
                mov     cx, ax

                pop     ax
                int     BIOS_INT_VIDEO  ; AH = BIOS_VIDEO_SET_MODE

                mov     ax, cx          ; return previous video mode
                ret


                global  VidDrawBitmap
VidDrawBitmap:
                push    dx              ; save image height
                push    cx              ; save image width

                mov     di, ax
                mov     cl, 8
                shr     di, cl          ; DI = x
                mov     ah, al
                mov     al, VID_CGA_HIMONO_LINE / 2
                mul     ah              ; AX = y * VID_CGA_HIMONO_LINE / 2
                add     di, ax          ; get the offset

                pop     dx              ; restore image width
                mov     cl, 3
                shr     dx, cl          ; get the image width in octets
                mov     cx, dx
                pop     dx              ; restore image height

                push    es              ; save and set the segment register
                mov     ax, VID_CGA_HIMONO_MEM     
                mov     es, ax
.Next:
                call    CgaDrawLine     ; draw even line
                xor     di, VID_CGA_HIMONO_PLANE
                dec     dx
  
                call    CgaDrawLine     ; draw odd line
                add     di, VID_CGA_HIMONO_LINE
                xor     di, VID_CGA_HIMONO_PLANE
                dec     dx
                jnz     .Next

                pop     es              ; restore the segment register
                ret


; Copy bitmap line to screen buffer
; Input:
;   DS:SI - bitmap line
;   ES:DI - screen line
;   CX    - number of octets
; Output:
;   DS:SI - next bitmap line
CgaDrawLine:
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

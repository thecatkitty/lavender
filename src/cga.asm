%define VID_API
%include "bios.inc"
%include "dos.inc"
%include "err.inc"
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


                global  VidGetPixelAspectRatio
VidGetPixelAspectRatio:
                push    bp
                mov     bp, sp
                sub     sp, VID_EDID_size
                mov     ax, (VID_EDID_TIMING_ASPECT_4_3 << VID_EDID_TIMING_ASPECT)

                mov     di, sp
                call    VesaReadEdid
                jc      .Convert
                mov     ax, word [bp - VID_EDID_size + VID_EDID.StandardTiming]
.Convert:
                mov     cl, VID_EDID_TIMING_ASPECT
                shr     ax, cl
                push    bx
                mov     bx, .LookupTable
                xlat
                pop     bx

                mov     sp, bp
                pop     bp
                ret
.LookupTable:
                db      VID_PAR(16,10,VID_CGA_HIMONO_WIDTH,VID_CGA_HIMONO_HEIGHT)
                db      VID_PAR(4,3,VID_CGA_HIMONO_WIDTH,VID_CGA_HIMONO_HEIGHT)
                db      VID_PAR(5,4,VID_CGA_HIMONO_WIDTH,VID_CGA_HIMONO_HEIGHT)
                db      VID_PAR(16,9,VID_CGA_HIMONO_WIDTH,VID_CGA_HIMONO_HEIGHT)


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
                call    CgaDrawBitmapLine
                xor     di, VID_CGA_HIMONO_PLANE
                dec     dx              ; even lines
  
                call    CgaDrawBitmapLine
                add     di, VID_CGA_HIMONO_LINE
                xor     di, VID_CGA_HIMONO_PLANE
                dec     dx              ; odd lines
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


; Read EDID record over VESA VBE/DC
; Input:
;   DS:DI - 128-byte output buffer
; Output:
;   DS:DI - EDID record
;   CF    - error
VesaReadEdid:
                push    ax
                push    bx
                push    cx
                push    dx
                push    es

                mov     ax, BIOS_VIDEO_VBE_DC
                mov     bl, BIOS_VIDEO_VBE_DC_CAPABILITIES
                int     BIOS_INT_VIDEO
                cmp     al, 4Fh
                jne     .Unsupported
                cmp     ah, 00h
                jne     .Failed

                mov     ax, BIOS_VIDEO_VBE_DC
                mov     bl, BIOS_VIDEO_VBE_DC_READ_EDID
                xor     cx, cx
                xor     dx, dx
                push    ds
                pop     es
                int     BIOS_INT_VIDEO
                cmp     al, 4Fh
                jne     .Unsupported
                cmp     ah, 00h
                je      .End
.Unsupported:
                ERR     VID_UNSUPPORTED
.Failed:
                ERR     VID_FAILED
.Error:
.End:           pop     es
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret

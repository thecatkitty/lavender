%define SLD_API
%include "gfx.inc"
%include "ker.inc"
%include "sld.inc"
%include "vid.inc"
%include "api/dos.inc"


                cpu     8086

[bits 16]
section .text

                global  SldEntryExecute
SldEntryExecute:
                ; Delay
                push    cx
                mov     cx, [di + SLD_ENTRY.wDelay]
                call    KerSleep
                pop     cx

                ; Type
                cmp     byte [di + SLD_ENTRY.bType], SLD_TYPE_TEXT
                je      SldEntryExecuteText
                cmp     byte [di + SLD_ENTRY.bType], SLD_TYPE_BITMAP
                je      SldEntryExecuteBitmap
                ret


SldEntryExecuteText:
                push    ax
                push    si

                ; Text - vertical position
                mov     al, [di + SLD_ENTRY.wVertical]

                ; Text - horizontal position
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ah, [di + SLD_ENTRY.wHorizontal]
                jmp     .Write
.AlignCenter:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.bLength]
                shr     ah, 1
                jmp     .Write
.AlignRight:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.bLength]
  
.Write:
                mov     si, di
                add     si, SLD_ENTRY.szContent
                call    VidDrawText
                clc

                pop     si
                pop     ax
                ret


SldEntryExecuteBitmap:
                push    ax
                push    bx
                push    cx
                push    dx
                push    si
                push    di

                ; Bitmap - locate file
                mov     bx, di
                add     bx, SLD_ENTRY.szContent
                xor     ch, ch
                mov     cl, byte [di + SLD_ENTRY.bLength]
                call    KerSearchArchive
                jc      .Error
                call    KerGetArchiveData               ; file data in DS:BX
                jc      .Error

                ; Bitmap - load image
                mov     si, bx
                mov     di, stPicture
                call    GfxLoadBitmap
                jc      .Error
                
                pop     bx
                xchg    di, bx

                ; Bitmap - horizontal position
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ax, word [di + SLD_ENTRY.wHorizontal]
                jmp     .Draw
.AlignCenter:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + GFX_BITMAP.wWidthBytes]
                shr     ax, 1
                jmp     .Draw
.AlignRight:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + GFX_BITMAP.wWidthBytes]
.Draw:
                mov     si, bx
                mov     bx, word [di + SLD_ENTRY.wVertical]
                call    VidDrawBitmap
                clc
                jmp     .End
.Error:
                pop     di
.End:           pop     si
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret


section .bss


stPicture                       resb    GFX_BITMAP_size

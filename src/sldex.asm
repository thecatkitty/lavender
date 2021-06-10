%define SLD_API
%include "dos.inc"
%include "ker.inc"
%include "pic.inc"
%include "sld.inc"
%include "vid.inc"
%include "zip.inc"


                cpu     8086

[bits 16]
section .text

                global  SldEntryExecute
SldEntryExecute:
                ; Delay
                push    cx
                mov     cx, [di + SLD_ENTRY.Delay]
                call    KerSleep
                pop     cx

                ; Type
                cmp     byte [di + SLD_ENTRY.Type], SLD_TYPE_TEXT
                je      SldEntryExecuteText
                cmp     byte [di + SLD_ENTRY.Type], SLD_TYPE_BITMAP
                je      SldEntryExecuteBitmap
                ret


SldEntryExecuteText:
                push    ax
                push    si

                ; Text - vertical position
                mov     al, [di + SLD_ENTRY.Vertical]

                ; Text - horizontal position
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ah, [di + SLD_ENTRY.Horizontal]
                jmp     .Write
.AlignCenter:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.Length]
                shr     ah, 1
                jmp     .Write
.AlignRight:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.Length]
  
.Write:
                mov     si, di
                add     si, SLD_ENTRY.Content
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
                add     bx, SLD_ENTRY.Content
                xor     ch, ch
                mov     cl, byte [di + SLD_ENTRY.Length]
                call    ZipLocateFileHeader
                jc      .End
                call    ZipLocateFileData               ; file data in DS:BX
                jc      .End

                ; Bitmap - load image
                mov     si, bx
                mov     di, oPicture
                call    PicLoadBitmap
                jc      .End
                
                pop     bx
                xchg    di, bx

                ; Bitmap - horizontal position
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ax, word [di + SLD_ENTRY.Horizontal]
                jmp     .Draw
.AlignCenter:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + PIC_BITMAP.WidthBytes]
                shr     ax, 1
                jmp     .Draw
.AlignRight:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + PIC_BITMAP.WidthBytes]
.Draw:
                mov     si, bx
                mov     bx, word [di + SLD_ENTRY.Vertical]
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


oPicture                        resb    PIC_BITMAP_size

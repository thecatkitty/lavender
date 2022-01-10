%define SLD_API
%include "err.inc"
%include "ker.inc"
%include "sld.inc"
%include "vid.inc"


                cpu     8086

[bits 16]
section .text


                global  SldEntryLoad
SldEntryLoad:
                cmp     byte [si], 0Dh
                je      .EndOfFile

                call    KerParseU16
                ERRC    SLD_INVALID_DELAY

                mov     dx, KER_DELAY_MS_MULTIPLIER
                mul     dx              ; DX:AX = AX * DX
                mov     bx, KER_DELAY_MS_DIVISOR
                div     bx              ; AX = DX:AX / BX
                mov     [di + SLD_ENTRY.wDelay], ax
  
                ; Get type
                cmp     byte [si], SLD_TAG_TYPE_TEXT
                je      .TypeText
                cmp     byte [si], SLD_TAG_TYPE_BITMAP
                je      .TypeBitmap
                ERR     SLD_UNKNOWN_TYPE

.TypeText:
                mov     byte [di + SLD_ENTRY.bType], SLD_TYPE_TEXT
                jmp    .TextBitmapCommon

.TypeBitmap:
                mov     byte [di + SLD_ENTRY.bType], SLD_TYPE_BITMAP

.TextBitmapCommon:
                call    SldLoadPosition
                jc      .Error

                call    SldLoadContent
                jnc     .End

.Error:         jmp     .End
.EndOfFile:
                mov     byte [di + SLD_ENTRY.bLength], 0
                clc
.End:           mov     ax, [di + SLD_ENTRY.bLength]
                ret


; Parse line row and column information from the slideshow file 
; Input:
;   DS:SI - beginning of the text field
;   DS:DI - SLD_ENTRY structure
SldLoadPosition:
                ; Parse row number
                inc     si
                call    KerParseU16
                ERRC    SLD_INVALID_VERTICAL
                mov     [di + SLD_ENTRY.wVertical], ax

                ; Parse column number
                call    KerParseU16
                jnc     .ColumnNumeric
                cmp     byte [si], SLD_TAG_ALIGN_LEFT
                je      .ColumnLeft
                cmp     byte [si], SLD_TAG_ALIGN_CENTER
                je      .ColumnCenter
                cmp     byte [si], SLD_TAG_ALIGN_RIGHT
                je      .ColumnRight
                ERR     SLD_INVALID_HORIZONTAL
  
.ColumnNumeric:
                mov     [di + SLD_ENTRY.wHorizontal], ax
                jmp     .End
  
.ColumnLeft:
                inc     si
                mov     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_LEFT
                jmp     .End
  
.ColumnCenter:
                inc     si
                mov     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_CENTER
                jmp     .End
  
.ColumnRight:
                inc     si
                mov     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_RIGHT
                jmp     .End

.Error:
.End:           ret


; Read line content from the slideshow file 
; Input:
;   DS:SI - slideshow file line content
;   DS:DI - SLD_ENTRY structure
; Output:
;   CF    - Error
SldLoadContent:
                push    dx
                push    ax
                push    di              ; save SLD_ENTRY structure pointer
                mov     byte [di + SLD_ENTRY.bLength], 0

                xor     dx, dx
                add     di, SLD_ENTRY.szContent
.Next:
                cmp     byte [si], 0Dh
                je      .Break

                push    bx              ; preserve registers
                push    cx
                push    dx
                mov     ax, wCodePoint
                push    ax              ; codePoint
                push    si              ; sequence
                call    KerGetCharacterFromUtf8
                add     sp, 4
                pop     dx              ; restore registers
                pop     cx
                pop     bx
                cmp     ax, 0           ; return value < 0
                jl      .Error
                add     si, ax
                mov     ax, word [wCodePoint]
                call    VidGetFontEncoding
                mov     byte [di], al
                inc     di
                inc     dx
                cmp     dx, SLD_ENTRY_MAX_LENGTH
                je      .Error
                jmp     .Next
.Break:
                mov     byte [di], 0
                pop     di
                mov     [di + SLD_ENTRY.bLength], dx
                add     si, 2
                jmp     .End

.Error:         pop     di
                stc
.End:           pop     ax
                pop     dx
                ret


section .bss


wCodePoint      resw    1

%define LINE_API
%include "line.inc"
%include "str.inc"


                cpu     8086

[bits 16]
section .text


                global  LineLoad
LineLoad:
                cmp     byte [si], 0Dh
                je      .EndOfFile

                ; Parse and convert delay
                jc      .Error
                call    StrParseU16
                mov     dx, LINE_DELAY_MS_MULTIPLIER
                mul     dx              ; DX:AX = AX * DX
                mov     bx, LINE_DELAY_MS_DIVISOR
                div     bx              ; AX = DX:AX / BX
                mov     [di + LINE.Delay], ax
  
                ; Get type
                cmp     byte [si], LINE_TAG_TYPE_TEXT
                je      .TypeText
                cmp     byte [si], LINE_TAG_TYPE_BITMAP
                je      .TypeBitmap
                jmp     .Error          ; unknown line type

.TypeText:
                mov     byte [di + LINE.Type], LINE_TYPE_TEXT
                jmp    .TextBitmapCommon

.TypeBitmap:
                mov     byte [di + LINE.Type], LINE_TYPE_BITMAP

.TextBitmapCommon:
                call    LineLoadPosition
                jc      .Error

                call    LineLoadContent
                jnc     .End

.Error:         stc
                jmp     .End
.EndOfFile:
                mov     byte [di + LINE.Length], 0
                clc
.End:           mov     ax, [di + LINE.Length]
                ret


; Parse line row and column information from the slideshow file 
; Input:
;   DS:SI - beginning of the text field
;   DS:DI - LINE structure
LineLoadPosition:
                ; Parse row number
                inc     si
                call    StrParseU16
                jc      .Error
                mov     [di + LINE.Vertical], ax

                ; Parse column number
                call    StrParseU16
                jnc     .ColumnNumeric
                cmp     byte [si], LINE_TAG_ALIGN_LEFT
                je      .ColumnLeft
                cmp     byte [si], LINE_TAG_ALIGN_CENTER
                je      .ColumnCenter
                cmp     byte [si], LINE_TAG_ALIGN_RIGHT
                je      .ColumnRight
                jmp     .Error
  
.ColumnNumeric:
                mov     [di + LINE.Horizontal], ax
                jmp     .End
  
.ColumnLeft:
                inc     si
                mov     word [di + LINE.Horizontal], LINE_ALIGN_LEFT
                jmp     .End
  
.ColumnCenter:
                inc     si
                mov     word [di + LINE.Horizontal], LINE_ALIGN_CENTER
                jmp     .End
  
.ColumnRight:
                inc     si
                mov     word [di + LINE.Horizontal], LINE_ALIGN_RIGHT
                jmp     .End

.Error:         stc
.End:           ret


; Read line content from the slideshow file 
; Input:
;   DS:SI - slideshow file line content
;   DS:DI - LINE structure
; Output:
;   CF    - Error
LineLoadContent:
                push    dx
                push    di              ; save LINE structure pointer
                mov     byte [di + LINE.Length], 0

                xor     dx, dx
                add     di, LINE.Content
.Next:
                mov     bl, [si]
                cmp     bl, 0Dh
                je      .Break
                mov     [di], bl
                inc     si
                inc     di
                inc     dx
                cmp     dx, LINE_MAX_LENGTH
                je      .Error
                jmp     .Next
.Break:
                mov     byte [di], 0
                pop     di
                mov     [di + LINE.Length], dx
                add     si, 2
                jmp     .End

.Error:         pop     di
                stc
.End:           pop     dx
                ret

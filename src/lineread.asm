%define LINE_API
%include "line.inc"
%include "str.inc"


                cpu     8086

[bits 16]
section .text


                global  LineLoad
LineLoad:
                cmp     byte [si], 0Dh
                je      .eof

  ; Parse and convert delay
                jc      .error
                call    StrParseU16
                mov     dx, 10          ; multiplier
                mul     dx              ; DX:AX = AX * 10
                mov     bx, 549         ; divisor
                div     bx              ; AX = DX:AX / 549
                mov     [di + LINE.Delay], ax
  
  ; Get type
                cmp     byte [si], 'T'
                je      .text_line
                cmp     byte [si], 'B'
                je      .bitmap_line
                jmp     .error          ; unknown line type

.text_line:
                mov     byte [di + LINE.Type], 0
                jmp    .text_bitmap_line_common

.bitmap_line:
                mov     byte [di + LINE.Type], 1

.text_bitmap_line_common:
                call    LineLoadPosition
                jc      .error

                call    LineLoadContent
                jnc     .end

.error:         stc
                jmp     .end
.eof:
                mov     byte [di + LINE.Length], 0
                clc
.end:           mov     ax, [di + LINE.Length]
                ret


; Parse line row and column information from the slideshow file 
; Input:
;   DS:SI - beginning of the text field
;   DS:DI - LINE structure
LineLoadPosition:
  ; Parse row number
                inc     si
                call    StrParseU16
                jc      .error
                mov     [di + LINE.Vertical], ax

  ; Parse column number
                call    StrParseU16
                jnc     .column_number
                cmp     byte [si], '<'
                je      .column_left
                cmp     byte [si], '^'
                je      .column_center
                cmp     byte [si], '>'
                je      .column_right
                jmp     .error
  
.column_number:
                mov     [di + LINE.Horizontal], ax
                jmp     .end
  
.column_left:
                inc     si
                mov     word [di + LINE.Horizontal], 0
                jmp     .end
  
.column_center:
                inc     si
                mov     word [di + LINE.Horizontal], 0FFF1h
                jmp     .end
  
.column_right:
                inc     si
                mov     word [di + LINE.Horizontal], 0FFF2h
                jmp     .end

.error:         stc
.end:           ret


; Read line content from the slideshow file 
; Input:
;   DS:SI - slideshow file line content
;   DS:DI - LINE structure
; Output:
;   CF    - error
LineLoadContent:
                push    dx
                push    di              ; save LINE structure pointer
                mov     byte [di + LINE.Length], 0

                xor     dx, dx
                add     di, LINE.Content
.next:
                mov     bl, [si]
                cmp     bl, 0Dh
                je      .break
                mov     [di], bl
                inc     si
                inc     di
                inc     dx
                cmp     dx, LINE_MAX_LENGTH
                je      .error
                jmp     .next
.break:
                mov     byte [di], 0
                pop     di
                mov     [di + LINE.Length], dx
                add     si, 2
                jmp     .end

.error:         pop     di
                stc
.end:           pop     dx
                ret

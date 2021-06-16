%define KER_API_SUBSET_UNICODE
%include "err.inc"
%include "ker.inc"


                cpu     8086

[bits 16]
section .text


                global  KerGetCharacterFromUtf8
KerGetCharacterFromUtf8:
                xor     ax, ax
                mov     ah, byte [si]
                test    ah, 10000000b
                jz      .SingleByte
                test    ah, 01000000b
                jnz     .Lead
                ERR     KER_INVALID_SEQUENCE
.SingleByte:
                xchg    al, ah
                inc     si
                jmp     .End
.Lead:
                test    ah, 00100000b
                jz      .TwoBytes
                ERR     KER_UNSUPPORTED_CODEPOINT
.TwoBytes:
                mov     al, byte [si + 1]
                and     al, 11000000b
                cmp     al, 10000000b
                jz      .LoadConvert
                ERR     KER_INVALID_SEQUENCE
.LoadConvert:
                mov     al, byte [si + 1]
                and     ah, 00011111b
                and     al, 00111111b
                shl     al, 1
                shl     al, 1
                shr     ax, 1
                shr     ax, 1
                add     si, 2
.Error:
.End:           ret


                global  KerCompareUtf8IgnoreCase
KerCompareUtf8IgnoreCase:
                push    di              ; STACK COMPOSITION, relative to BP
                push    si              ;  +12  - preserved DI
                push    dx              ;  +10  - preserved SI
                push    cx              ;   +8  - preserved DX
                push    bx              ;   +6  - preserved CX
                push    ax              ;   +4  - preserved BX
                push    bp              ;   +2  - preserved AX
                mov     bp, sp          ;   -0  - preserved BP
                                        ;       - buffer of size 6 * CX
                ; convert left string
                mov     dx, cx
                shl     dx, 1 
                add     dx, cx
                shl     dx, 1           ; DX = 6 * CX
                sub     sp, dx          ; prepare local stack space 

                mov     dx, cx
                add     dx, si          ; DX = end of the left string

                mov     di, sp          ; use local stack space
.LeftNext:
                cmp     si, dx
                jae     .Compare        ; end of the left string

                call    KerGetCharacterFromUtf8
                jc      .Error
                call    KerFoldCase

                mov     word [di], ax   ; save first case-folded code point
                add     di, 2
                test    bx, bx
                jz      .LeftNext
                mov     word [di], bx   ; save second case-folded code point
                add     di, 2
                jmp     .LeftNext
.Compare:
                mov     di, sp          ; rewind to the start of left buffer
                mov     si, word [bp+4] ; load preserved BX
                mov     dx, word [bp+6] ; load preserved CX
                add     dx, si          ; DX = end of the second string
.RightNext:
                cmp     si, dx
                jae     .SameStrings

                call    KerGetCharacterFromUtf8
                jc      .Error
                call    KerFoldCase

                cmp     ax, word [di]
                jne     .Return
                add     di, 2
                test    bx, bx
                jz      .RightNext
                cmp     bx, word [di]
                jne     .Return
                add     di, 2
                jmp     .RightNext

.SameStrings:   cmp     ax, ax
.Return:        clc
                jmp     .End
.Error:
.End:           
                mov     sp, bp
                pop     bp
                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     si
                pop     di
                ret


                global  KerFoldCase
KerFoldCase:
%push Context

%macro          JBOIN   5

                cmp     %1, %2
                jb      %4
                cmp     %1, %3
                jbe     %5

%endmacro

                xor     bx, bx

                cmp     ax, 07FFh       ; current maximum code point
                ja      .End

; LATIN CAPITAL LETTER A -- LATIN CAPITAL LETTER Z
                JBOIN   ax, 0041h, 005Ah, .End, .Add32
; LATIN CAPITAL LETTER A WITH GRAVE -- LATIN CAPITAL LETTER THORN
                JBOIN   ax, 00C0h, 00DEh, .End, .Add32
; LATIN CAPITAL LETTER A WITH MACRON -- LATIN SMALL LETTER I WITH OGONEK
                JBOIN   ax, 0100h, 012Fh, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LIGATURE IJ -- LATIN SMALL LETTER K WITH CEDILLA
                JBOIN   ax, 0132h, 0137h, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LETTER L WITH ACUTE -- LATIN SMALL LETTER N WITH CARON
                JBOIN   ax, 0139h, 0148h, .CheckSingle, .OddCapitals
; LATIN CAPITAL LETTER ENG -- LATIN SMALL LETTER Y WITH CIRCUMFLEX
                JBOIN   ax, 014Ah, 0177h, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LETTER A WITH CARON -- LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE
                JBOIN   ax, 01CDh, 01DCh, .CheckSingle, .OddCapitals
; LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON -- LATIN SMALL LETTER EZH WITH CARON
                JBOIN   ax, 01DEh, 01EFh, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LETTER N WITH GRAVE -- LATIN SMALL LETTER H WITH CARON
                JBOIN   ax, 01F8h, 021Fh, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LETTER OU -- LATIN SMALL LETTER Y WITH MACRON
                JBOIN   ax, 0222h, 0233h, .CheckSingle, .EvenCapitals
; LATIN CAPITAL LETTER E WITH STROKE -- LATIN SMALL LETTER Y WITH STROKE
                JBOIN   ax, 0246h, 024Fh, .CheckSingle, .EvenCapitals

.CheckSingle:
                mov     bx, .awcSingleCharacters
.NextSingle:
                cmp     word [bx], ax
                ja      .CheckDouble
                je      .Single
                add     bx, 2 * 2
                jmp     .NextSingle
.Single:
                mov     ax, word [bx + 1 * 2]
                xor     bx, bx
                jmp     .End

.CheckDouble:
                mov     bx, .awcDoubleCharacters
.NextDouble:
                cmp     word [bx], ax
                ja      .Identity
                je      .Double
                add     bx, 3 * 2
                jmp     .NextDouble
.Double:
                mov     ax, word [bx + 1 * 2]
                mov     bx, word [bx + 2 * 2]
                jmp     .End

.EvenCapitals:
                or      ax, 1
                jmp     .End
.OddCapitals:
                test    ax, 1
                jz      .End
                add     ax, 1
                jmp     .End
.Add32:
                add     ax, 32
.Identity:
                xor     bx, bx
.End:
                ret

.awcSingleCharacters:
                dw      00B5h, 03BCh    ; MICRO SIGN
                dw      0178h, 00FFh    ; LATIN CAPITAL LETTER Y WITH DIAERESIS
                dw      0179h, 017Ah    ; LATIN CAPITAL LETTER Z WITH ACUTE
                dw      017Bh, 017Ch    ; LATIN CAPITAL LETTER Z WITH DOT ABOVE
                dw      017Dh, 017Eh    ; LATIN CAPITAL LETTER Z WITH CARON
                dw      017Fh, 0073h    ; LATIN SMALL LETTER LONG S
                dw      0181h, 0253h    ; LATIN CAPITAL LETTER B WITH HOOK
                dw      0182h, 0183h    ; LATIN CAPITAL LETTER B WITH TOPBAR
                dw      0184h, 0185h    ; LATIN CAPITAL LETTER TONE SIX
                dw      0186h, 0254h    ; LATIN CAPITAL LETTER OPEN O
                dw      0187h, 0188h    ; LATIN CAPITAL LETTER C WITH HOOK
                dw      0189h, 0256h    ; LATIN CAPITAL LETTER AFRICAN D
                dw      018Ah, 0257h    ; LATIN CAPITAL LETTER D WITH HOOK
                dw      018Bh, 018Ch    ; LATIN CAPITAL LETTER D WITH TOPBAR
                dw      018Eh, 01DDh    ; LATIN CAPITAL LETTER REVERSED E
                dw      018Fh, 0259h    ; LATIN CAPITAL LETTER SCHWA
                dw      0190h, 025Bh    ; LATIN CAPITAL LETTER OPEN E
                dw      0191h, 0192h    ; LATIN CAPITAL LETTER F WITH HOOK
                dw      0193h, 0260h    ; LATIN CAPITAL LETTER G WITH HOOK
                dw      0194h, 0263h    ; LATIN CAPITAL LETTER GAMMA
                dw      0196h, 0269h    ; LATIN CAPITAL LETTER IOTA
                dw      0197h, 0268h    ; LATIN CAPITAL LETTER I WITH STROKE
                dw      0198h, 0199h    ; LATIN CAPITAL LETTER K WITH HOOK
                dw      019Ch, 026Fh    ; LATIN CAPITAL LETTER TURNED M
                dw      019Dh, 0272h    ; LATIN CAPITAL LETTER N WITH LEFT HOOK
                dw      019Fh, 0275h    ; LATIN CAPITAL LETTER O WITH MIDDLE TILDE
                dw      01A0h, 01A1h    ; LATIN CAPITAL LETTER O WITH HORN
                dw      01A2h, 01A3h    ; LATIN CAPITAL LETTER OI
                dw      01A4h, 01A5h    ; LATIN CAPITAL LETTER P WITH HOOK
                dw      01A6h, 0280h    ; LATIN LETTER YR
                dw      01A7h, 01A8h    ; LATIN CAPITAL LETTER TONE TWO
                dw      01A9h, 0283h    ; LATIN CAPITAL LETTER ESH
                dw      01ACh, 01ADh    ; LATIN CAPITAL LETTER T WITH HOOK
                dw      01AEh, 0288h    ; LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
                dw      01AFh, 01B0h    ; LATIN CAPITAL LETTER U WITH HORN
                dw      01B1h, 028Ah    ; LATIN CAPITAL LETTER UPSILON
                dw      01B2h, 028Bh    ; LATIN CAPITAL LETTER V WITH HOOK
                dw      01B3h, 01B4h    ; LATIN CAPITAL LETTER Y WITH HOOK
                dw      01B5h, 01B6h    ; LATIN CAPITAL LETTER Z WITH STROKE
                dw      01B7h, 0292h    ; LATIN CAPITAL LETTER EZH
                dw      01B8h, 01B9h    ; LATIN CAPITAL LETTER EZH REVERSED
                dw      01BCh, 01BDh    ; LATIN CAPITAL LETTER TONE FIVE
                dw      01C4h, 01C6h    ; LATIN CAPITAL LETTER DZ WITH CARON
                dw      01C5h, 01C6h    ; LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
                dw      01C7h, 01C9h    ; LATIN CAPITAL LETTER LJ
                dw      01C8h, 01C9h    ; LATIN CAPITAL LETTER L WITH SMALL LETTER J
                dw      01CAh, 01CCh    ; LATIN CAPITAL LETTER NJ
                dw      01F1h, 01F3h    ; LATIN CAPITAL LETTER DZ
                dw      01F2h, 01F3h    ; LATIN CAPITAL LETTER D WITH SMALL LETTER Z
                dw      01F4h, 01F5h    ; LATIN CAPITAL LETTER G WITH ACUTE
                dw      01F6h, 0195h    ; LATIN CAPITAL LETTER HWAIR
                dw      01F7h, 01BFh    ; LATIN CAPITAL LETTER WYNN
                dw      0220h, 019Eh    ; LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
                dw      023Ah, 2C65h    ; LATIN CAPITAL LETTER A WITH STROKE
                dw      023Bh, 023Ch    ; LATIN CAPITAL LETTER C WITH STROKE
                dw      023Dh, 019Ah    ; LATIN CAPITAL LETTER L WITH BAR
                dw      023Eh, 2C66h    ; LATIN CAPITAL LETTER T WITH DIAGONAL STROKE
                dw      0241h, 0242h    ; LATIN CAPITAL LETTER GLOTTAL STOP
                dw      0243h, 0180h    ; LATIN CAPITAL LETTER B WITH STROKE
                dw      0244h, 0289h    ; LATIN CAPITAL LETTER U BAR
                dw      0245h, 028Ch    ; LATIN CAPITAL LETTER TURNED V
                dw      0FFFFh
.awcDoubleCharacters:
                dw      00DFh, 0073h, 0073h     ; LATIN SMALL LETTER SHARP S
                dw      0130h, 0069h, 0307h     ; LATIN CAPITAL LETTER I WITH DOT ABOVE
                dw      0149h, 02BCh, 006Eh     ; LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
                dw      01F0h, 006Ah, 030Ch     ; LATIN SMALL LETTER J WITH CARON
                dw      0FFFFh
%pop

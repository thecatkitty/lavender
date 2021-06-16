; Video memory base
%define CGA_HIMONO_MEM                  0B800h

; Odd plane offset
%define CGA_HIMONO_PLANE                2000h

; Text mode character height
%define CGA_CHARACTER_HEIGHT            8

; IVT pointer to font data over code point 127
%define INT_CGA_EXTENDED_FONT_PTR       31

; 0FFA6Eh - ROM font base
%define CGA_BASIC_FONT_SEGMENT          0FFA0h
%define CGA_BASIC_FONT_OFFSET           6Eh

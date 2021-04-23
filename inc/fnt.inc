%ifndef FNT_API

%define FNT_API extern

; Initialize extended screen font
FNT_API FntLoad

; Deinitialize extended screen font
FNT_API FntUnload

%endif

FNT_INTV_EXTENDED_FONT_PTR      equ     31              ; index in the IVT
FNT_BASIC_FONT_SEGMENT          equ     0FFA0h          ; 0FFA6Eh - ROM font base
FNT_BASIC_FONT_OFFSET           equ     6Eh
FNT_CHARACTER_HEIGHT            equ     8

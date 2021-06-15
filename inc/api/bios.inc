BIOS_INT_VIDEO                  equ     10h
BIOS_VIDEO_SET_MODE             equ     00h             ; AH
BIOS_VIDEO_SET_CURSOR_POSITION  equ     02h             ; AH
BIOS_VIDEO_GET_MODE             equ     0Fh             ; AH
BIOS_VIDEO_VBE_DC               equ     4F15h           ; AX
BIOS_VIDEO_VBE_DC_CAPABILITIES  equ     00H             ; BL
BIOS_VIDEO_VBE_DC_READ_EDID     equ     01H             ; BL

BIOS_INT_KEYBOARD               equ     16h
BIOS_KEYBOARD_GET_KEYSTROKE     equ     00h             ; AH

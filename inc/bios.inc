BIOS_INT_VIDEO                  equ     10h
BIOS_VIDEO_SET_MODE             equ     00h             ; AH
BIOS_VIDEO_SET_CURSOR_POSITION  equ     02h             ; AH
BIOS_VIDEO_GET_MODE             equ     0Fh             ; AH

BIOS_INT_KEYBOARD               equ     16h
BIOS_KEYBOARD_GET_KEYSTROKE     equ     00h             ; AH

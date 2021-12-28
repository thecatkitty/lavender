%define BIOS_INT_VIDEO                  10h
%define BIOS_VIDEO_SET_MODE             00h             ; AH
%define BIOS_VIDEO_SET_CURSOR_POSITION  02h             ; AH
%define BIOS_VIDEO_GET_MODE             0Fh             ; AH
%define BIOS_VIDEO_VBE_DC               4F15h           ; AX
%define BIOS_VIDEO_VBE_DC_CAPABILITIES  00H             ; BL
%define BIOS_VIDEO_VBE_DC_READ_EDID     01H             ; BL

%define BIOS_INT_KEYBOARD               16h
%define BIOS_KEYBOARD_GET_KEYSTROKE     00h             ; AH

%ifndef ERR_INC
%define ERR_INC

%define ERR_FACILITY_KER                0
%define ERR_FACILITY_VID                1
%define ERR_FACILITY_GFX                2
%define ERR_FACILITY_SLD                3

%define ERR_OK                          0

%define ERR_CODE(facility,code)         (((facility) << 5) | ((code) & 11111b))

%macro          ERR     1

                stc
                mov     byte [KerLastError], ERR_%1
                jmp     .Error

%endmacro

%macro          ERRC    1

                jnc     %%Skip
                mov     byte [KerLastError], ERR_%1
                jmp     .Error
%%Skip:

%endmacro

%endif ; ERR_INC

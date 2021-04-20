%ifndef ERR_API

%define ERR_API extern

; Print error message and terminate program execution
ERR_API ErrTerminate

; Byte variable for storing error code
ERR_API ErrLastCode

%endif

ERR_OK                          equ     0
ERR_LINE_INVALID_DELAY          equ     1
ERR_LINE_UNKNOWN_TYPE           equ     2
ERR_LINE_INVALID_VERTICAL       equ     3
ERR_LINE_INVALID_HORIZONTAL     equ     4

%macro          ERR     1

                stc
                mov     byte [ErrLastCode], ERR_%1
                jmp     .Error

%endmacro

%macro          ERRC    1

                jnc     %%Skip
                mov     byte [ErrLastCode], ERR_%1
                jmp     .Error
%%Skip:

%endmacro

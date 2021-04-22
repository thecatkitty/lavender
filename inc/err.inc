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
ERR_ZIP_CDIR_END_NOT_FOUND      equ     5
ERR_ZIP_CDIR_TOO_LARGE          equ     6
ERR_ZIP_CDIR_INVALID            equ     7
ERR_ZIP_NOT_FOUND               equ     8
ERR_ZIP_LOCAL_INVALID           equ     9
ERR_ZIP_UNSUPPORTED_METHOD      equ     10
ERR_ZIP_UNSUPPORTED_FLAGS       equ     11


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

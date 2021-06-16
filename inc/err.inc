%ifndef ERR_API

%define ERR_API extern

; Print error message and terminate program execution
ERR_API KerTerminate

; Byte variable for storing error code
ERR_API KerLastError

%endif

ERR_OK                          equ     0
ERR_SLD_INVALID_DELAY           equ     1
ERR_SLD_UNKNOWN_TYPE            equ     2
ERR_SLD_INVALID_VERTICAL        equ     3
ERR_SLD_INVALID_HORIZONTAL      equ     4
ERR_ZIP_CDIR_END_NOT_FOUND      equ     5
ERR_ZIP_CDIR_TOO_LARGE          equ     6
ERR_ZIP_CDIR_INVALID            equ     7
ERR_ZIP_NOT_FOUND               equ     8
ERR_ZIP_LOCAL_INVALID           equ     9
ERR_ZIP_UNSUPPORTED_METHOD      equ     10
ERR_ZIP_UNSUPPORTED_FLAGS       equ     11
ERR_UNI_INVALID_SEQUENCE        equ     12
ERR_UNI_UNSUPPORTED_CODEPOINT   equ     13
ERR_PIC_UNSUPPORTED_FORMAT      equ     14
ERR_PIC_MALFORMED_FILE          equ     15
ERR_VID_UNSUPPORTED             equ     16
ERR_VID_FAILED                  equ     17
ERR_VID_FORMAT                  equ     18


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

%define KER_API_SUBSET_ERROR
%include "err.inc"
%include "ker.inc"
%include "api/dos.inc"


                cpu     8086

[bits 16]
section .text

                global  KerTerminate
KerTerminate:
                mov     ah, DOS_PUTS
                mov     dx, sErrHeader
                int     DOS_INT

                xor     cx, cx
                mov     cl, [KerLastError]
                mov     si, asErrMessages
.NextMessage:
                cmp     byte [si], cl
                je      .PrintMessage
.NextCharacter:
                inc     si
                cmp     byte [si], '$'
                jne     .NextCharacter
                inc     si
                jmp     .NextMessage
.PrintMessage:
                inc     si
                mov     dx, si
                int     DOS_INT

                mov     al, [KerLastError]
                mov     ah, DOS_EXIT
                int     DOS_INT


section .data


sErrHeader                      db      "ERROR: $"
asErrMessages                   db      ERR_OK, "OK$"
                                db      ERR_KER_UNSUPPORTED,        "Kernel - Unsupported feature requested$"
                                db      ERR_KER_NOT_FOUND,          "Kernel - Item not found$"
                                db      ERR_KER_ARCHIVE_NOT_FOUND,  "Kernel - Archive not found$"
                                db      ERR_KER_ARCHIVE_TOO_LARGE,  "Kernel - Archive is too large$"
                                db      ERR_KER_ARCHIVE_INVALID,    "Kernel - Archive is invalid$"
                                db      ERR_KER_INVALID_SEQUENCE,   "Kernel - Invalid or unsupported UTF-8 sequence$"
                                db      ERR_VID_UNSUPPORTED,        "Video - Unsupported feature requested$"
                                db      ERR_VID_FAILED,             "Video - Operation failed$"
                                db      ERR_VID_FORMAT,             "Video - Improper graphics format$"
                                db      ERR_GFX_FORMAT,             "Graphics - Unsupported format$"
                                db      ERR_GFX_MALFORMED_FILE,     "Graphics - Malformed file$"
                                db      ERR_SLD_INVALID_DELAY,      "Slides - Invalid delay$"
                                db      ERR_SLD_UNKNOWN_TYPE,       "Slides - Unknown type$"
                                db      ERR_SLD_INVALID_VERTICAL,   "Slides - Invalid vertical position$"
                                db      ERR_SLD_INVALID_HORIZONTAL, "Slides - Invalid horizontal position$"


section .bss


                                global  KerLastError
KerLastError                    resb    1

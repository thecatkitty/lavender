%define ERR_API
%include "dos.inc"
%include "err.inc"


                cpu     8086

[bits 16]
section .text

                global  ErrTerminate
ErrTerminate:
                mov     ah, DOS_PUTS
                mov     dx, sErrHeader
                int     DOS_INT

                xor     cx, cx
                mov     cl, [ErrLastCode]
                mov     si, asErrMessages
                jcxz    .PrintMessage
.NextCharacter:
                cmp     byte [si], '$'
                je      .NextMessage
                inc     si
                jmp     .NextCharacter
.NextMessage:
                inc     si
                dec     cx
                jz      .PrintMessage
                jmp     .NextCharacter
.PrintMessage:
                mov     dx, si
                int     DOS_INT

                mov     al, [ErrLastCode]
                mov     ah, DOS_EXIT
                int     DOS_INT


section .data


sErrHeader                      db      "ERROR: $"
asErrMessages                   db      "OK$"
                                db      "Slides - Invalid delay$"
                                db      "Slides - Unknown type$"
                                db      "Slides - Invalid vertical position$"
                                db      "Slides - Invalid horizontal position$"
                                db      "ZIP - Central directory not found$"
                                db      "ZIP - Central directory is too large$"
                                db      "ZIP - Central directory is invalid$"
                                db      "ZIP - File not found$"
                                db      "ZIP - Local file header is invalid$"
                                db      "ZIP - Compression method not supported$"
                                db      "ZIP - File flags require unsupported features$"
                                db      "Unicode - Invalid UTF-8 sequence$"
                                db      "Unicode - Unsupported code point$"
                                db      "Picture - Unsupported format$"
                                db      "Picture - Malformed file$"

section .bss

                                global  ErrLastCode
ErrLastCode                     resb    1

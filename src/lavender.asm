%include "bios.inc"
%include "dos.inc"
%include "err.inc"
%include "line.inc"
%include "vid.inc"
%include "zip.inc"


                cpu     8086

                extern  ArchiveStart
                extern  ArchiveEnd

[bits 16]
section .init


                jmp     LavenderEntry


section .text


LavenderEntry:
                mov     ax, VID_MODE_CGA_HIMONO
                call    VidSetMode
                push    ax              ; save previous mode on stack

                mov     bx, ArchiveStart
                mov     si, ArchiveEnd
                call    ZipLocateCentralDirectoryEnd
                jc      .Error


                mov     bx, sLogo
                mov     cx, 12
                call    ZipLocateFileHeader
                jc      .Error

                push    si
                mov     si, di
                add     si, ZIP_LOCAL_FILE_HEADER_size
                add     si, [di + ZIP_LOCAL_FILE_HEADER.NameLength]
                add     si, [di + ZIP_LOCAL_FILE_HEADER.ExtraLength]
                add     si, 57          ; TODO : replace after PBM implementation
                mov     ah, (640 - LOGOW) / 2 / 8
                mov     al, (144 - LOGOH) / 2
                mov     cx, LOGOW
                mov     dx, LOGOH
                call    VidDrawBitmap
                pop     si


                mov     bx, sSlides
                mov     cx, 10
                call    ZipLocateFileHeader
                jc      .Error

                mov     si, di
                add     si, ZIP_LOCAL_FILE_HEADER_size
                add     si, [di + ZIP_LOCAL_FILE_HEADER.NameLength]
                add     si, [di + ZIP_LOCAL_FILE_HEADER.ExtraLength]
.Next:
                mov     di, oLine
                call    LineLoad
                jc      .Error
                test    ax, ax
                jz      .End
                push    si

                call    LineExecute
                pop     si
                jmp    .Next

.End:
                xor     ah, ah          ; AH = BIOS_KEYBOARD_GET_KEYSTROKE
                int     BIOS_INT_KEYBOARD

                pop     ax              ; restore saved mode
                call    VidSetMode

                mov     ax, (DOS_EXIT << 8 | ERR_OK)
                int     DOS_INT
.Error:
                call    ErrTerminate


section .data


LOGOW                           equ     272             ; logo width
LOGOH                           equ     100             ; logo height

sLogo                           db      "cgihisym.pbm"
sSlides                         db      "slides.txt"


section .bss


oLine                           resb    LINE_size

%include "bios.inc"
%include "dos.inc"
%include "err.inc"
%include "fnt.inc"
%include "line.inc"
%include "vid.inc"
%include "zip.inc"


                cpu     8086

                extern  ArchiveStart
                extern  ArchiveEnd
                extern  StackBottom

[bits 16]
section .init


                mov     sp, StackBottom
                jmp     LavenderEntry
%push About
%defstr GIT_COMMIT      %!GIT_COMMIT
%defstr GIT_TAG         %!GIT_TAG
                db      0Dh, "Lavender ", GIT_TAG, '-', GIT_COMMIT, 1Ah
%pop


section .text


LavenderEntry:
                mov     ax, VID_MODE_CGA_HIMONO
                call    VidSetMode
                push    ax              ; save previous mode on stack

                call FntLoad

                mov     bx, ArchiveStart
                mov     si, ArchiveEnd
                call    ZipLocateCentralDirectoryEnd
                jc      .Error


                mov     bx, sLogo
                mov     cx, lLogo
                call    ZipLocateFileHeader
                jc      .Error
                call    ZipLocateFileData
                jc      .Error

                push    si
                mov     si, bx
                add     si, 57          ; TODO : replace after PBM implementation
                mov     ah, (640 - LOGOW) / 2 / 8
                mov     al, (144 - LOGOH) / 2
                mov     cx, LOGOW
                mov     dx, LOGOH
                call    VidDrawBitmap
                pop     si


                mov     bx, sSlides
                mov     cx, lSlides
                call    ZipLocateFileHeader
                jc      .Error
                call    ZipLocateFileData
                jc      .Error

                mov     si, bx
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

                call    FntUnload

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
lLogo                           equ     $ - sLogo
sSlides                         db      "slides.txt"
lSlides                         equ     $ - sSlides


section .bss


oLine                           resb    LINE_size

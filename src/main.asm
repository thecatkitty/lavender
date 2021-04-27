%include "bios.inc"
%include "err.inc"
%include "fnt.inc"
%include "ker.inc"
%include "sld.inc"
%include "vid.inc"
%include "zip.inc"


                cpu     8086

                extern  ArchiveStart
                extern  ArchiveEnd

[bits 16]
section .text

                global  Main
Main:
                mov     ax, VID_MODE_CGA_HIMONO
                call    VidSetMode
                push    ax              ; save previous mode on stack

                call    FntLoad

                mov     bx, ArchiveStart
                mov     si, ArchiveEnd
                call    ZipLocateCentralDirectoryEnd
                jc      ErrTerminate


                mov     bx, sLogo
                mov     cx, lLogo
                call    ZipLocateFileHeader
                jc      ErrTerminate
                call    ZipLocateFileData
                jc      ErrTerminate

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
                jc      ErrTerminate
                call    ZipLocateFileData
                jc      ErrTerminate

                mov     si, bx
.Next:
                mov     di, oEntry
                call    SldEntryLoad
                jc      ErrTerminate
                test    ax, ax
                jz      .End
                push    si

                call    SldEntryExecute
                pop     si
                jmp    .Next

.End:
                xor     ah, ah          ; AH = BIOS_KEYBOARD_GET_KEYSTROKE
                int     BIOS_INT_KEYBOARD

                call    FntUnload

                pop     ax              ; restore saved mode
                call    VidSetMode

                xor     al, al          ; AL = ERR_OK
                ret


section .data


LOGOW                           equ     272             ; logo width
LOGOH                           equ     100             ; logo height

sLogo                           db      "cgihisym.pbm"
lLogo                           equ     $ - sLogo
sSlides                         db      "slides.txt"
lSlides                         equ     $ - sSlides


section .bss


oEntry                          resb    SLD_ENTRY_size

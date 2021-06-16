%include "bios.inc"
%include "dos.inc"
%include "err.inc"
%include "fnt.inc"
%include "ker.inc"
%include "pic.inc"
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

                call    VidLoadFont

                mov     bx, ArchiveStart
                mov     si, ArchiveEnd
                call    KerLocateArchive
                jc      KerTerminate
                mov     word [pstDirectoryEnd], si

                mov     bx, sSlides
                mov     cx, nSlidesLength
                call    KerSearchArchive
                jc      KerTerminate
                call    KerGetArchiveData
                jc      KerTerminate

                mov     si, bx
.Next:
                mov     di, stEntry
                call    SldEntryLoad
                jc      KerTerminate
                test    ax, ax
                jz      .End
                push    si

                mov     si, word [pstDirectoryEnd]
                call    SldEntryExecute
                pop     si
                jc      KerTerminate
                jmp    .Next

.End:
                xor     ah, ah          ; AH = BIOS_KEYBOARD_GET_KEYSTROKE
                int     BIOS_INT_KEYBOARD

                call    VidUnloadFont

                pop     ax              ; restore saved mode
                call    VidSetMode

                xor     al, al          ; AL = ERR_OK
                ret


section .data


sSlides                         db      "slides.txt"
nSlidesLength                   equ     $ - sSlides


section .bss


pstDirectoryEnd                 resw    1
stEntry                         resb    SLD_ENTRY_size

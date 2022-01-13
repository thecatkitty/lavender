%include "ker.inc"
%include "sld.inc"
%include "vid.inc"


                cpu     8086

                extern  __edata

[bits 16]
section .text

                global  Main
Main:
                mov     ax, VID_MODE_CGA_HIMONO
                push    ax
                call    VidSetMode
                add     sp, 2
                push    ax              ; save previous mode on stack

                call    VidLoadFont

                mov     ax, pstDirectoryEnd 
                push    ax              ; cdir
                mov     ax, 0FD00h
                push    ax              ; to
                mov     ax, __edata
                push    ax              ; from
                call    KerLocateArchive
                add     sp, 6
                cmp     ax, 0
                jl      KerTerminate

                mov     ax, pstLocalFile    
                push    ax              ; lfh
                mov     cx, nSlidesLength
                push    cx              ; nameLength
                mov     ax, sSlides         
                push    ax              ; name
                mov     ax, word [pstDirectoryEnd]
                push    ax              ; cdir
                call    KerSearchArchive
                add     sp, 8
                cmp     ax, 0
                jl      KerTerminate

                mov     ax, pabFileData
                push    ax              ; data
                mov     ax, word [pstLocalFile]
                push    ax              ; lfh
                call    KerGetArchiveData
                add     sp, 4
                cmp     ax, 0
                jl      KerTerminate

                mov     si, word [pabFileData]
.Next:
                mov     di, stEntry
                push    di              ; out
                push    si              ; line
                call    SldLoadEntry
                add     sp, 4
                cmp     ax, 0
                jl      KerTerminate
                test    ax, ax
                jz      .End
                add     si, ax
                push    si

                mov     di, stEntry
                mov     si, word [pstDirectoryEnd]
                call    SldEntryExecute
                pop     si
                jc      KerTerminate
                jmp    .Next

.End:
                xor     ah, ah          ; get key stroke
                int     16h

                call    VidUnloadFont

                call    VidSetMode      ; restore saved mode
                add     sp, 2

                xor     al, al          ; AL = ERR_OK
                ret


section .data


sSlides                         db      "slides.txt"
nSlidesLength                   equ     $ - sSlides


section .bss


pstDirectoryEnd                 resw    1
pstLocalFile                    resw    1
pabFileData                     resw    1
stEntry                         resb    SLD_ENTRY_size

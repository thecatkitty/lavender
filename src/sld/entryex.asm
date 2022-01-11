%define SLD_API
%include "gfx.inc"
%include "ker.inc"
%include "sld.inc"
%include "vid.inc"
%include "api/dos.inc"


                cpu     8086

[bits 16]
section .text

                global  SldEntryExecute
SldEntryExecute:
                ; Delay
                push    ax
                push    bx
                push    cx
                push    dx
                push    word [di + SLD_ENTRY.wDelay]
                call    KerSleep
                add     sp, 2
                pop     dx
                pop     cx
                pop     bx
                pop     ax

                ; Type
                cmp     byte [di + SLD_ENTRY.bType], SLD_TYPE_TEXT
                je      SldEntryExecuteText
                cmp     byte [di + SLD_ENTRY.bType], SLD_TYPE_BITMAP
                je      SldEntryExecuteBitmap
                ret


SldEntryExecuteText:
                push    ax
                push    si

                ; Text - vertical position
                mov     al, [di + SLD_ENTRY.wVertical]

                ; Text - horizontal position
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ah, [di + SLD_ENTRY.wHorizontal]
                jmp     .Write
.AlignCenter:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.bLength]
                shr     ah, 1
                jmp     .Write
.AlignRight:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.bLength]
  
.Write:
                mov     si, di
                add     si, SLD_ENTRY.szContent
                call    VidDrawText
                clc

                pop     si
                pop     ax
                ret


SldEntryExecuteBitmap:
                push    ax
                push    bx
                push    cx
                push    dx
                push    si
                push    di

                ; Bitmap - locate file
                mov     bx, di
                add     bx, SLD_ENTRY.szContent
                xor     ch, ch
                mov     cl, byte [di + SLD_ENTRY.bLength]
                call    SldLocateBestBitmap
                jc      .Error
                
                push    ax
                mov     bx, pabFileData
                push    bx              ; data
                mov     bx, word [pstLocalFile]
                push    bx              ; lfh
                call    KerGetArchiveData
                add     sp, 4
                cmp     ax, 0
                pop     ax
                jl      .Error

                ; Bitmap - load image
                mov     si, word [pabFileData]
                mov     di, stPicture
                call    GfxLoadBitmap
                jc      .Error
                
                pop     bx
                xchg    di, bx

                ; Bitmap - horizontal position
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.wHorizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ax, word [di + SLD_ENTRY.wHorizontal]
                jmp     .Draw
.AlignCenter:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + GFX_BITMAP.wWidthBytes]
                shr     ax, 1
                jmp     .Draw
.AlignRight:
                mov     ax, VID_CGA_HIMONO_LINE
                sub     ax, word [bx + GFX_BITMAP.wWidthBytes]
.Draw:
                mov     si, bx
                mov     bx, word [di + SLD_ENTRY.wVertical]
                call    VidDrawBitmap
                clc
                jmp     .End
.Error:
                pop     di
.End:           pop     si
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret


; Locate the most fitting bitmap for a given file name
; Input:
;   DS:SI - ZIP central directory end structure
;   DS:BX - file name
;   CX    - file name length
; Output:
;   DS:DI - ZIP local file header structure
;   CF    - error
; Invalidates: BX, CX
SldLocateBestBitmap:
                push    ax              ; STACK COMPOSITION, relative to BP
                push    bp              ;  +2  - preserved AX
                mov     bp, sp          ;  -0  - preserved BP
                sub     sp, 4           ;  -2  - file name begin pointer
                                        ;  -4  - field begin pointer
                mov     word [bp - 2], bx
                mov     ax, bx
                add     ax, cx
                dec     ax
.FindPattern:
                cmp     byte [bx], '<'
                je      .CheckPattern
                inc     bx
                cmp     bx, ax
                je      .NoPattern
                jmp     .FindPattern
.CheckPattern:
                cmp     byte [bx + 1], '>'
                jne     .FindPattern
                mov     word [bp - 4], bx

                ; Pixel Aspect Ratio field
                xor     ax, ax
                call    VidGetPixelAspectRatio          ; AX - reference PAR
                mov     bx, word [bp - 2]               ; BX - file name
                mov     dx, word [bp - 4]               ; CX - file name length
                call    SldFindBitmapNearestPar         ; DX - pixel aspect ration file name field
                jc      .End
.NoPattern:
                push    ax
                mov     bx, pstLocalFile    
                push    bx              ; lfh
                push    cx              ; nameLength
                mov     bx, word [bp - 2]         
                push    bx              ; name
                push    si              ; cdir
                call    KerSearchArchive
                add     sp, 8
                cmp     ax, 0
                pop     ax
                jl      .Error
                mov     di, word [pstLocalFile]
                jmp     .End
.Error:
                stc
.End:
                mov     sp, bp
                pop     bp
                pop     ax
                ret


; Locate the bitmap with the nearest PAR
; Input:
;   DS:SI - ZIP central directory end structure
;   AX    - reference PAR
;   DS:BX - file name
;   CX    - file name length
;   DS:DX - pixel aspect ration file name field
; Output:
;   DS:DI - ZIP local file header structure
;   CF    - error
; Invalidates: AX, BX, CX
SldFindBitmapNearestPar:
                push    bp              ; STACK COMPOSITION, relative to BP
                mov     bp, sp          ;  -0  - preserved BP
                sub     sp, 8           ;  -2  - current decreasing PAR
                                        ;  -4  - current increasing PAR
                                        ;  -6  - file name
                                        ;  -8  - file name length
                mov     word [bp - 2], ax
                mov     word [bp - 4], ax
                inc     word [bp - 2]
                mov     word [bp - 6], bx
                mov     word [bp - 8], cx
.CheckDecreasing:
                mov     ax, word [bp - 2]
                cmp     ax, 0
                jz      .CheckIncreasing
                call    .Check
                jnc     .End
                dec     word [bp - 2]
.CheckIncreasing:
                mov     ax, word [bp - 4]
                cmp     ax, 0FFh
                je      .CheckEnd
                call    .Check
                jnc     .End
                inc     word [bp - 4]
.CheckEnd:
                cmp     word [bp - 2], 0
                jnz     .CheckDecreasing
                stc
.End:
                mov     sp, bp
                pop     bp
                ret
.Check:
                call    KerByteToHex
                xchg    al, ah
                xchg    bx, dx
                mov     word [bx], ax                   ; replace pattern with number
                xchg    bx, dx
                
                push    ax
                push    bx
                push    cx
                push    dx
                mov     bx, pstLocalFile    
                push    bx              ; lfh
                mov     cx, word [bp - 8]
                push    cx              ; nameLength
                mov     bx, word [bp - 6]         
                push    bx              ; name
                push    si              ; cdir
                call    KerSearchArchive
                add     sp, 8
                cmp     ax, 0
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                jl      .CheckError
                mov     di, word [pstLocalFile]
                ret
.CheckError:
                stc
                ret


section .bss


stPicture                       resb    GFX_BITMAP_size
pstLocalFile                    resw    1
pabFileData                     resw    1

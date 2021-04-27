%define SLD_API
%include "ker.inc"
%include "sld.inc"
%include "vid.inc"


                cpu     8086

[bits 16]
section .text

                global  SldEntryExecute
SldEntryExecute:
                ; Delay
                mov     cx, [di + SLD_ENTRY.Delay]
                call    KerSleep

                ; Type
                cmp     byte [di + SLD_ENTRY.Type], SLD_TYPE_TEXT
                jne     .End

                ; Text - vertical position
                mov     al, [di + SLD_ENTRY.Vertical]

                ; Text - horizontal position
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_CENTER
                je      .AlignCenter
                cmp     word [di + SLD_ENTRY.Horizontal], SLD_ALIGN_RIGHT
                je      .AlignRight
                mov     ah, [di + SLD_ENTRY.Horizontal]
                jmp     .Write
.AlignCenter:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.Length]
                shr     ah, 1
                jmp     .Write
.AlignRight:
                mov     ah, SLD_ENTRY_MAX_LENGTH
                sub     ah, [di + SLD_ENTRY.Length]
  
.Write:
                mov     si, di
                add     si, SLD_ENTRY.Content
                call    VidDrawText

.End:           ret

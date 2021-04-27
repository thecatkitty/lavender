%define KER_API
%include "dos.inc"
%include "ker.inc"


                cpu     8086

                extern  Main
                extern  StackBottom
                extern  StackTop

[bits 16]
section .init


                mov     sp, StackBottom
                call    Main
                mov     ah, DOS_EXIT
                int     DOS_INT
%push About
%defstr GIT_COMMIT      %!GIT_COMMIT
%defstr GIT_TAG         %!GIT_TAG
                db      0Dh, "Lavender ", GIT_TAG, '-', GIT_COMMIT, 1Ah
%pop


section .text

                global  KerSleep
KerSleep:
                jcxz    .End
.Next:
                hlt
                loop    .Next

.End:           ret

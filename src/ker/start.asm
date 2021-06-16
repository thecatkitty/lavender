%define KER_API_SUBSET_START
%include "ker.inc"
%include "api/dos.inc"


                cpu     8086

                extern  Main
                extern  StackBottom
                extern  StackTop

                extern  PitInitialize
                extern  PitDeinitialize


[bits 16]
section .init


                jmp     KerEntry
%push About
%defstr GIT_COMMIT      %!GIT_COMMIT
%defstr GIT_TAG         %!GIT_TAG
                db      0Dh, "Lavender ", GIT_TAG, '-', GIT_COMMIT, 1Ah
%pop


section .text


KerEntry:
                ; Initialize stack
                mov     sp, StackBottom

                call    PitInitialize

                call    Main
                ; Pass to KerExit


KerExit:
                call    PitDeinitialize

                mov     ah, DOS_EXIT
                int     DOS_INT

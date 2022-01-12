                cpu     8086
                
%defstr GIT_COMMIT      %!GIT_COMMIT
%defstr GIT_TAG         %!GIT_TAG

                extern  Main
                extern  PitInitialize
                extern  PitDeinitialize


[bits 16]
section .startupA


                global  _start
_start:
                jmp     KerEntry
                db      0Dh, "Lavender ", GIT_TAG, '-', GIT_COMMIT, 1Ah


section .text


KerEntry:
                call    PitInitialize

                call    Main

                call    PitDeinitialize

                mov     ah, 4Ch         ; Exit gracefully
                int     21h


section .ctors.errf


                global  ErrFacilities
ErrFacilities:


section .ctors.errm


                global  ErrMessages
ErrMessages:

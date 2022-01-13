%include "err.inc"

%define KER_API extern


; ----- ZIP archive functions -----

; Locate ZIP central directory end structure
; Stack input:
;   search range lower boundary (near)
;   search range upper boundary (near)
;   near pointer to output structure pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerLocateArchive

; Locate ZIP local file header structure
; Stack input:
;   near pointer to ZIP central directory end structure
;   near pointer to the file name
;   file name length
;   near pointer to output structure pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerSearchArchive

; Locate ZIP file data
; Stack input:
;   near pointer to ZIP local file header structure
;   near pointer to ZIP file data pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerGetArchiveData


; ----- Error handling functions -----

; Print error message and terminate program execution
KER_API KerTerminate

; Byte variable for storing error code
KER_API KerLastError


%define ERR_KER_UNSUPPORTED             ERR_CODE(ERR_FACILITY_KER, 1)
%define ERR_KER_NOT_FOUND               ERR_CODE(ERR_FACILITY_KER, 2)
%define ERR_KER_ARCHIVE_NOT_FOUND       ERR_CODE(ERR_FACILITY_KER, 3)
%define ERR_KER_ARCHIVE_TOO_LARGE       ERR_CODE(ERR_FACILITY_KER, 4)
%define ERR_KER_ARCHIVE_INVALID         ERR_CODE(ERR_FACILITY_KER, 5)
%define ERR_KER_INVALID_SEQUENCE        ERR_CODE(ERR_FACILITY_KER, 6)

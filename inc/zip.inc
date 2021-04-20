%ifndef ZIP_API

%define ZIP_API extern

; Locate ZIP central directory end structure
; Input:
;   DS:BX - search range lower boundary
;   DS:SI - search range upper boundary
; Output:
;   DS:SI - ZIP central directory end structure
;   CF    - error
; Invalidates:
ZIP_API ZipLocateCentralDirectoryEnd

; Locate ZIP local file header structure
; Input:
;   DS:SI - ZIP central directory end structure
;   DS:BX - file name
;   CX    - file name length
; Output:
;   DS:DI - ZIP local file header structure
;   CF    - error
; Invalidates: BX, CX
ZIP_API ZipLocateFileHeader

%endif

ZIP_PK_SIGN                     equ     4B50h
ZIP_LOCAL_FILE_SIGN             equ     0403h
ZIP_CDIR_FILE_SIGN              equ     0201h
ZIP_CDIR_END_SIGN               equ     0605h

struc ZIP_CDIR_END_HEADER
  .PkSignature:                 resw    1
  .HeaderSignature:             resw    1
  .DiskNumber:                  resw    1
  .CentralDirectoryDisk:        resw    1
  .DiskEntries:                 resw    1
  .TotalEntries:                resw    1
  .CentralDirectorySize:        resd    1
  .CentralDirectoryOffset:      resd    1
  .CommentLength:               resw    1
  .Comment:
endstruc

struc ZIP_CDIR_FILE_HEADER
  .PkSignature:                 resw    1
  .HeaderSignature:             resw    1
  .Version:                     resw    1
  .VersionNeeded:               resw    1
  .Flags:                       resw    1
  .Compression:                 resw    1
  .ModificationTime:            resw    1
  .ModificationDate:            resw    1
  .Crc32:                       resd    1
  .CompressedSize:              resd    1
  .UncompressedSize:            resd    1
  .NameLength:                  resw    1
  .ExtraLength:                 resw    1
  .CommentLength:               resw    1
  .StartDiskNumber:             resw    1
  .InternalAttributes:          resw    1
  .ExternalAttributes:          resd    1
  .LocalHeaderOffset:           resd    1
  .Name:
endstruc

struc ZIP_LOCAL_FILE_HEADER
  .PkSignature:                 resw    1
  .HeaderSignature:             resw    1
  .Version:                     resw    1
  .Flags:                       resw    1
  .Compression:                 resw    1
  .ModificationTime:            resw    1
  .ModificationDate:            resw    1
  .Crc32:                       resd    1
  .CompressedSize:              resd    1
  .UncompressedSize:            resd    1
  .NameLength:                  resw    1
  .ExtraLength:                 resw    1
  .Name:
endstruc

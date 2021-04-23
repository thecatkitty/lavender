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

; Locate ZIP file data
; Input:
;   DS:DI - ZIP local file header structure
; Output:
;   DS:BX - ZIP file data
;   CF    - error
ZIP_API ZipLocateFileData

%endif

ZIP_PK_SIGN                     equ     4B50h
ZIP_LOCAL_FILE_SIGN             equ     0403h
ZIP_CDIR_FILE_SIGN              equ     0201h
ZIP_CDIR_END_SIGN               equ     0605h

ZIP_VERSION_FS_MSDOS            equ     0
ZIP_VERSION_FS_AMIGA            equ     1
ZIP_VERSION_FS_OPENVMS          equ     2
ZIP_VERSION_FS_UNIX             equ     3
ZIP_VERSION_FS_VMCMS            equ     4
ZIP_VERSION_FS_ATARIST          equ     5
ZIP_VERSION_FS_HPFS             equ     6
ZIP_VERSION_FS_MACINTOSH        equ     7
ZIP_VERSION_FS_ZSYSTEM          equ     8
ZIP_VERSION_FS_CPM              equ     9
ZIP_VERSION_FS_NTFS             equ     10
ZIP_VERSION_FS_MVS              equ     11
ZIP_VERSION_FS_VSE              equ     12
ZIP_VERSION_FS_ACORN            equ     13
ZIP_VERSION_FS_VFAT             equ     14
ZIP_VERSION_FS_ALTMVS           equ     15
ZIP_VERSION_FS_BEOS             equ     16
ZIP_VERSION_FS_TANDEM           equ     17
ZIP_VERSION_FS_OS400            equ     18
ZIP_VERSION_FS_DARWIN           equ     19

ZIP_FLAG_ENCRYPTED              equ     0000000000000001b
ZIP_FLAG_COMPRESSION_OPTIONS    equ     0000000000000110b
ZIP_FLAG_FIELDS_IN_DDESC        equ     0000000000001000b
ZIP_FLAG_ENHANCED_DEFLATE       equ     0000000000010000b
ZIP_FLAG_PATCHED_DATA           equ     0000000000100000b
ZIP_FLAG_STRONG_ENCRYPTION      equ     0000000001000000b
ZIP_FLAG_LANGUAGE_ENCODING      equ     0000100000000000b
ZIP_FLAG_MASK_HEADER_VALUES     equ     0010000000000000b
ZIP_FLAGS_SUPPORTED             equ     (ZIP_FLAG_LANGUAGE_ENCODING)

ZIP_METHOD_STORE                equ     0
ZIP_METHOD_SHRINK               equ     1
ZIP_METHOD_REDUCE1              equ     2
ZIP_METHOD_REDUCE2              equ     3
ZIP_METHOD_REDUCE3              equ     4
ZIP_METHOD_REDUCE4              equ     5
ZIP_METHOD_IMPLODE              equ     6
ZIP_METHOD_DEFLATE              equ     8
ZIP_METHOD_DEFLATE64            equ     9
ZIP_METHOD_PKWAREDCL            equ     10
ZIP_METHOD_BZIP2                equ     12
ZIP_METHOD_LZMA                 equ     14
ZIP_METHOD_IBMCMPSC             equ     16
ZIP_METHOD_IBMTERSE             equ     18
ZIP_METHOD_IBMLZ77              equ     19
ZIP_METHOD_ZSTD                 equ     93
ZIP_METHOD_MP3                  equ     94
ZIP_METHOD_XZ                   equ     95
ZIP_METHOD_JPEG                 equ     96
ZIP_METHOD_WAVPACK              equ     97
ZIP_METHOD_PPMD                 equ     98
ZIP_METHOD_AEX                  equ     99

ZIP_TIME_TWOSECONDS             equ     000000000001111b
ZIP_TIME_TWOSECONDS_SHIFT       equ     0
ZIP_TIME_MINUTES                equ     000001111110000b
ZIP_TIME_MINUTES_SHIFT          equ     4
ZIP_TIME_HOURS                  equ     111110000000000b
ZIP_TIME_HOURS_SHIFT            equ     10

ZIP_TIME_DAYS                   equ     000000000011111b
ZIP_TIME_DAYS_SHIFT             equ     0
ZIP_TIME_MONTHS                 equ     000000111100000b
ZIP_TIME_MONTHS_SHIFT           equ     5
ZIP_TIME_YEARS                  equ     111111000000000b
ZIP_TIME_YEARS_SHIFT            equ     9
ZIP_TIME_YEARS_BASE             equ     1980

ZIP_EXTRA_INFOZIP_UNICODE_PATH  equ     7075h

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

struc ZIP_EXTRA_FIELD_HEADER
  .Signature:                   resw    1
  .TotalSize:                   resw    1
endstruc

struc ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD
  .Signature:                   resw    1
  .TotalSize:                   resw    1
  .Version:                     resb    1
  .NameCrc32:                   resd    1
  .UnicodeName:
endstruc

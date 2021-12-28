%define ZIP_PK_SIGN                     4B50h
%define ZIP_LOCAL_FILE_SIGN             0403h
%define ZIP_CDIR_FILE_SIGN              0201h
%define ZIP_CDIR_END_SIGN               0605h

%define ZIP_VERSION_FS_MSDOS            0
%define ZIP_VERSION_FS_AMIGA            1
%define ZIP_VERSION_FS_OPENVMS          2
%define ZIP_VERSION_FS_UNIX             3
%define ZIP_VERSION_FS_VMCMS            4
%define ZIP_VERSION_FS_ATARIST          5
%define ZIP_VERSION_FS_HPFS             6
%define ZIP_VERSION_FS_MACINTOSH        7
%define ZIP_VERSION_FS_ZSYSTEM          8
%define ZIP_VERSION_FS_CPM              9
%define ZIP_VERSION_FS_NTFS             10
%define ZIP_VERSION_FS_MVS              11
%define ZIP_VERSION_FS_VSE              12
%define ZIP_VERSION_FS_ACORN            13
%define ZIP_VERSION_FS_VFAT             14
%define ZIP_VERSION_FS_ALTMVS           15
%define ZIP_VERSION_FS_BEOS             16
%define ZIP_VERSION_FS_TANDEM           17
%define ZIP_VERSION_FS_OS400            18
%define ZIP_VERSION_FS_DARWIN           19

%define ZIP_FLAG_ENCRYPTED              0000000000000001b
%define ZIP_FLAG_COMPRESSION_OPTIONS    0000000000000110b
%define ZIP_FLAG_FIELDS_IN_DDESC        0000000000001000b
%define ZIP_FLAG_ENHANCED_DEFLATE       0000000000010000b
%define ZIP_FLAG_PATCHED_DATA           0000000000100000b
%define ZIP_FLAG_STRONG_ENCRYPTION      0000000001000000b
%define ZIP_FLAG_LANGUAGE_ENCODING      0000100000000000b
%define ZIP_FLAG_MASK_HEADER_VALUES     0010000000000000b
%define ZIP_FLAGS_SUPPORTED             (ZIP_FLAG_LANGUAGE_ENCODING)

%define ZIP_METHOD_STORE                0
%define ZIP_METHOD_SHRINK               1
%define ZIP_METHOD_REDUCE1              2
%define ZIP_METHOD_REDUCE2              3
%define ZIP_METHOD_REDUCE3              4
%define ZIP_METHOD_REDUCE4              5
%define ZIP_METHOD_IMPLODE              6
%define ZIP_METHOD_DEFLATE              8
%define ZIP_METHOD_DEFLATE64            9
%define ZIP_METHOD_PKWAREDCL            10
%define ZIP_METHOD_BZIP2                12
%define ZIP_METHOD_LZMA                 14
%define ZIP_METHOD_IBMCMPSC             16
%define ZIP_METHOD_IBMTERSE             18
%define ZIP_METHOD_IBMLZ77              19
%define ZIP_METHOD_ZSTD                 93
%define ZIP_METHOD_MP3                  94
%define ZIP_METHOD_XZ                   95
%define ZIP_METHOD_JPEG                 96
%define ZIP_METHOD_WAVPACK              97
%define ZIP_METHOD_PPMD                 98
%define ZIP_METHOD_AEX                  99

%define ZIP_TIME_TWOSECONDS             000000000001111b
%define ZIP_TIME_TWOSECONDS_SHIFT       0
%define ZIP_TIME_MINUTES                000001111110000b
%define ZIP_TIME_MINUTES_SHIFT          4
%define ZIP_TIME_HOURS                  111110000000000b
%define ZIP_TIME_HOURS_SHIFT            10

%define ZIP_TIME_DAYS                   000000000011111b
%define ZIP_TIME_DAYS_SHIFT             0
%define ZIP_TIME_MONTHS                 000000111100000b
%define ZIP_TIME_MONTHS_SHIFT           5
%define ZIP_TIME_YEARS                  111111000000000b
%define ZIP_TIME_YEARS_SHIFT            9
%define ZIP_TIME_YEARS_BASE             1980

%define ZIP_EXTRA_INFOZIP_UNICODE_PATH  7075h

struc ZIP_CDIR_END_HEADER
  .wPkSignature:                resw    1
  .wHeaderSignature:            resw    1
  .wDiskNumber:                 resw    1
  .wCentralDirectoryDisk:       resw    1
  .wDiskEntries:                resw    1
  .wTotalEntries:               resw    1
  .dwCentralDirectorySize:      resd    1
  .dwCentralDirectoryOffset:    resd    1
  .wCommentLength:              resw    1
  .sComment:
endstruc

struc ZIP_CDIR_FILE_HEADER
  .wPkSignature:                resw    1
  .wHeaderSignature:            resw    1
  .wVersion:                    resw    1
  .wVersionNeeded:              resw    1
  .wFlags:                      resw    1
  .wCompression:                resw    1
  .wModificationTime:           resw    1
  .wModificationDate:           resw    1
  .dwCrc32:                     resd    1
  .dwCompressedSize:            resd    1
  .dwUncompressedSize:          resd    1
  .wNameLength:                 resw    1
  .wExtraLength:                resw    1
  .wCommentLength:              resw    1
  .wStartDiskNumber:            resw    1
  .wInternalAttributes:         resw    1
  .dwExternalAttributes:        resd    1
  .dwLocalHeaderOffset:         resd    1
  .sName:
endstruc

struc ZIP_LOCAL_FILE_HEADER
  .wPkSignature:                resw    1
  .wHeaderSignature:            resw    1
  .wVersion:                    resw    1
  .wFlags:                      resw    1
  .wCompression:                resw    1
  .wModificationTime:           resw    1
  .wModificationDate:           resw    1
  .dwCrc32:                     resd    1
  .dwCompressedSize:            resd    1
  .dwUncompressedSize:          resd    1
  .wNameLength:                 resw    1
  .wExtraLength:                resw    1
  .sName:
endstruc

struc ZIP_EXTRA_FIELD_HEADER
  .wSignature:                  resw    1
  .wTotalSize:                  resw    1
endstruc

struc ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD
  .wSignature:                  resw    1
  .wTotalSize:                  resw    1
  .wVersion:                    resb    1
  .dwNameCrc32:                 resd    1
  .sUnicodeName:
endstruc

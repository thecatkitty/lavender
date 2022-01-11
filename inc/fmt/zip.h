#ifndef _FMT_ZIP_H_
#define _FMT_ZIP_H_

#include <stdint.h>

#define ZIP_PK_SIGN                     0x4B50
#define ZIP_LOCAL_FILE_SIGN             0x0403
#define ZIP_CDIR_FILE_SIGN              0x0201
#define ZIP_CDIR_END_SIGN               0x0605

#define ZIP_VERSION_FS_MSDOS            0
#define ZIP_VERSION_FS_AMIGA            1
#define ZIP_VERSION_FS_OPENVMS          2
#define ZIP_VERSION_FS_UNIX             3
#define ZIP_VERSION_FS_VMCMS            4
#define ZIP_VERSION_FS_ATARIST          5
#define ZIP_VERSION_FS_HPFS             6
#define ZIP_VERSION_FS_MACINTOSH        7
#define ZIP_VERSION_FS_ZSYSTEM          8
#define ZIP_VERSION_FS_CPM              9
#define ZIP_VERSION_FS_NTFS             10
#define ZIP_VERSION_FS_MVS              11
#define ZIP_VERSION_FS_VSE              12
#define ZIP_VERSION_FS_ACORN            13
#define ZIP_VERSION_FS_VFAT             14
#define ZIP_VERSION_FS_ALTMVS           15
#define ZIP_VERSION_FS_BEOS             16
#define ZIP_VERSION_FS_TANDEM           17
#define ZIP_VERSION_FS_OS400            18
#define ZIP_VERSION_FS_DARWIN           19

#define ZIP_FLAG_ENCRYPTED              0b0000000000000001
#define ZIP_FLAG_COMPRESSION_OPTIONS    0b0000000000000110
#define ZIP_FLAG_FIELDS_IN_DDESC        0b0000000000001000
#define ZIP_FLAG_ENHANCED_DEFLATE       0b0000000000010000
#define ZIP_FLAG_PATCHED_DATA           0b0000000000100000
#define ZIP_FLAG_STRONG_ENCRYPTION      0b0000000001000000
#define ZIP_FLAG_LANGUAGE_ENCODING      0b0000100000000000
#define ZIP_FLAG_MASK_HEADER_VALUES     0b0010000000000000
#define ZIP_FLAGS_SUPPORTED             (ZIP_FLAG_LANGUAGE_ENCODING)

#define ZIP_METHOD_STORE                0
#define ZIP_METHOD_SHRINK               1
#define ZIP_METHOD_REDUCE1              2
#define ZIP_METHOD_REDUCE2              3
#define ZIP_METHOD_REDUCE3              4
#define ZIP_METHOD_REDUCE4              5
#define ZIP_METHOD_IMPLODE              6
#define ZIP_METHOD_DEFLATE              8
#define ZIP_METHOD_DEFLATE64            9
#define ZIP_METHOD_PKWAREDCL            10
#define ZIP_METHOD_BZIP2                12
#define ZIP_METHOD_LZMA                 14
#define ZIP_METHOD_IBMCMPSC             16
#define ZIP_METHOD_IBMTERSE             18
#define ZIP_METHOD_IBMLZ77              19
#define ZIP_METHOD_ZSTD                 93
#define ZIP_METHOD_MP3                  94
#define ZIP_METHOD_XZ                   95
#define ZIP_METHOD_JPEG                 96
#define ZIP_METHOD_WAVPACK              97
#define ZIP_METHOD_PPMD                 98
#define ZIP_METHOD_AEX                  99

#define ZIP_TIME_TWOSECONDS             0b000000000001111
#define ZIP_TIME_TWOSECONDS_SHIFT       0
#define ZIP_TIME_MINUTES                0b000001111110000
#define ZIP_TIME_MINUTES_SHIFT          4
#define ZIP_TIME_HOURS                  0b111110000000000
#define ZIP_TIME_HOURS_SHIFT            10

#define ZIP_TIME_DAYS                   0b000000000011111
#define ZIP_TIME_DAYS_SHIFT             0
#define ZIP_TIME_MONTHS                 0b000000111100000
#define ZIP_TIME_MONTHS_SHIFT           5
#define ZIP_TIME_YEARS                  0b111111000000000
#define ZIP_TIME_YEARS_SHIFT            9
#define ZIP_TIME_YEARS_BASE             1980

#define ZIP_EXTRA_INFOZIP_UNICODE_PATH  0x7075

typedef struct {
  uint16_t PkSignature;
  uint16_t HeaderSignature;
  uint16_t DiskNumber;
  uint16_t CentralDirectoryDisk;
  uint16_t DiskEntries;
  uint16_t TotalEntries;
  uint32_t CentralDirectorySize;
  uint32_t CentralDirectoryOffset;
  uint16_t CommentLength;
  char     Comment[];
} ZIP_CDIR_END_HEADER;

typedef struct {
  uint16_t PkSignature;
  uint16_t HeaderSignature;
  uint16_t Version;
  uint16_t VersionNeeded;
  uint16_t Flags;
  uint16_t Compression;
  uint16_t ModificationTime;
  uint16_t ModificationDate;
  uint32_t Crc32;
  uint32_t CompressedSize;
  uint32_t UncompressedSize;
  uint16_t NameLength;
  uint16_t ExtraLength;
  uint16_t CommentLength;
  uint16_t StartDiskNumber;
  uint16_t InternalAttributes;
  uint32_t ExternalAttributes;
  uint32_t LocalHeaderOffset;
  char     Name[];
} ZIP_CDIR_FILE_HEADER;

typedef struct {
  uint16_t PkSignature;
  uint16_t HeaderSignature;
  uint16_t Version;
  uint16_t Flags;
  uint16_t Compression;
  uint16_t ModificationTime;
  uint16_t ModificationDate;
  uint32_t Crc32;
  uint32_t CompressedSize;
  uint32_t UncompressedSize;
  uint16_t NameLength;
  uint16_t ExtraLength;
  char     Name[];
} ZIP_LOCAL_FILE_HEADER;

typedef struct {
  uint16_t Signature;
  uint16_t TotalSize;
} ZIP_EXTRA_FIELD_HEADER;

typedef struct {
  uint16_t Signature;
  uint16_t TotalSize;
  uint16_t Version;
  uint32_t NameCrc32;
  char     UnicodeName[];
} ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD;

#endif // _FMT_ZIP_H_

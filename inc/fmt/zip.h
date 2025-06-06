#ifndef _FMT_ZIP_H_
#define _FMT_ZIP_H_

#include <assert.h>
#include <stdio.h>

#include <base.h>

#define ZIP_PK_SIGN         0x4B50
#define ZIP_LOCAL_FILE_SIGN 0x0403
#define ZIP_CDIR_FILE_SIGN  0x0201
#define ZIP_CDIR_END_SIGN   0x0605

#define ZIP_VERSION_FS_MSDOS     0
#define ZIP_VERSION_FS_AMIGA     1
#define ZIP_VERSION_FS_OPENVMS   2
#define ZIP_VERSION_FS_UNIX      3
#define ZIP_VERSION_FS_VMCMS     4
#define ZIP_VERSION_FS_ATARIST   5
#define ZIP_VERSION_FS_HPFS      6
#define ZIP_VERSION_FS_MACINTOSH 7
#define ZIP_VERSION_FS_ZSYSTEM   8
#define ZIP_VERSION_FS_CPM       9
#define ZIP_VERSION_FS_NTFS      10
#define ZIP_VERSION_FS_MVS       11
#define ZIP_VERSION_FS_VSE       12
#define ZIP_VERSION_FS_ACORN     13
#define ZIP_VERSION_FS_VFAT      14
#define ZIP_VERSION_FS_ALTMVS    15
#define ZIP_VERSION_FS_BEOS      16
#define ZIP_VERSION_FS_TANDEM    17
#define ZIP_VERSION_FS_OS400     18
#define ZIP_VERSION_FS_DARWIN    19

#define ZIP_FLAG_ENCRYPTED           (1 << 0)
#define ZIP_FLAG_COMPRESSION_OPTIONS (3 << 1)
#define ZIP_FLAG_FIELDS_IN_DDESC     (1 << 3)
#define ZIP_FLAG_ENHANCED_DEFLATE    (1 << 4)
#define ZIP_FLAG_PATCHED_DATA        (1 << 5)
#define ZIP_FLAG_STRONG_ENCRYPTION   (1 << 6)
#define ZIP_FLAG_LANGUAGE_ENCODING   (1 << 11)
#define ZIP_FLAG_MASK_HEADER_VALUES  (1 << 13)
#define ZIP_FLAGS_SUPPORTED          (ZIP_FLAG_LANGUAGE_ENCODING)

#define ZIP_METHOD_STORE     0
#define ZIP_METHOD_SHRINK    1
#define ZIP_METHOD_REDUCE1   2
#define ZIP_METHOD_REDUCE2   3
#define ZIP_METHOD_REDUCE3   4
#define ZIP_METHOD_REDUCE4   5
#define ZIP_METHOD_IMPLODE   6
#define ZIP_METHOD_DEFLATE   8
#define ZIP_METHOD_DEFLATE64 9
#define ZIP_METHOD_PKWAREDCL 10
#define ZIP_METHOD_BZIP2     12
#define ZIP_METHOD_LZMA      14
#define ZIP_METHOD_IBMCMPSC  16
#define ZIP_METHOD_IBMTERSE  18
#define ZIP_METHOD_IBMLZ77   19
#define ZIP_METHOD_ZSTD      93
#define ZIP_METHOD_MP3       94
#define ZIP_METHOD_XZ        95
#define ZIP_METHOD_JPEG      96
#define ZIP_METHOD_WAVPACK   97
#define ZIP_METHOD_PPMD      98
#define ZIP_METHOD_AEX       99

#define ZIP_TIME_TWOSECONDS       0b000000000001111
#define ZIP_TIME_TWOSECONDS_SHIFT 0
#define ZIP_TIME_MINUTES          0b000001111110000
#define ZIP_TIME_MINUTES_SHIFT    4
#define ZIP_TIME_HOURS            0b111110000000000
#define ZIP_TIME_HOURS_SHIFT      10

#define ZIP_TIME_DAYS         0b000000000011111
#define ZIP_TIME_DAYS_SHIFT   0
#define ZIP_TIME_MONTHS       0b000000111100000
#define ZIP_TIME_MONTHS_SHIFT 5
#define ZIP_TIME_YEARS        0b111111000000000
#define ZIP_TIME_YEARS_SHIFT  9
#define ZIP_TIME_YEARS_BASE   1980

#define ZIP_EXTRA_INFOZIP_UNICODE_PATH 0x7075

#pragma pack(push, 1)

typedef struct
{
    uint16_t pk_signature;
    uint16_t header_signature;
    uint16_t disk_number;
    uint16_t cdir_disk;
    uint16_t disk_entries;
    uint16_t total_entries;
    uint32_t cdir_size;
    uint32_t cdir_offset;
    uint16_t comment_length;
    char     comment[];
} zip_cdir_end_header;
static_assert(22 == sizeof(zip_cdir_end_header),
              "Central directory end header size doesn't match specification");

typedef struct
{
    uint16_t pk_signature;
    uint16_t header_signature;
    uint16_t version;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint16_t modification_time;
    uint16_t modification_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t name_length;
    uint16_t extra_length;
    uint16_t comment_length;
    uint16_t start_disk_number;
    uint16_t internal_attrs;
    uint32_t external_attrs;
    uint32_t lfh_offset;
    char     name[];
} zip_cdir_file_header;
static_assert(46 == sizeof(zip_cdir_file_header),
              "Central directory file header size doesn't match specification");

typedef struct
{
    uint16_t pk_signature;
    uint16_t header_signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modification_time;
    uint16_t modification_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t name_length;
    uint16_t extra_length;
    char     name[];
} zip_local_file_header;
static_assert(30 == sizeof(zip_local_file_header),
              "Local file header size doesn't match specification");

typedef struct
{
    uint16_t signature;
    uint16_t total_size;
} zip_extra_fields_header;
static_assert(4 == sizeof(zip_extra_fields_header),
              "Extra fields header size doesn't match specification");

typedef struct
{
    uint16_t signature;
    uint16_t total_size;
    uint16_t version;
    uint32_t name_crc32;
    char     unicode_name[];
} zip_extra_unicode_path_field;
static_assert(10 == sizeof(zip_extra_unicode_path_field),
              "Extra Unicode path field size doesn't match specification");

#pragma pack(pop)

#ifdef CONFIG_COMPACT
typedef zip_cdir_end_header *zip_archive;
#else
typedef const char *zip_archive;
#endif

typedef off_t zip_item;
typedef long  zip_cached;

typedef bool (*zip_enum_files_callback)(zip_cdir_file_header *cfh, void *data);

// Set working archive
// archive is either a pointer to file name, or to address of ZIP central
// directory
extern bool
zip_open(zip_archive archive);

// Close the working archive
extern void
zip_close(void);

// Enumerate all files present in the archive
int
zip_enum_files(zip_enum_files_callback callback, void *data);

// Locate ZIP local file header structure
extern zip_item
zip_search(const char *name, uint16_t length);

// Retrieve ZIP file data
// Returns NULL on error
extern char *
zip_load_data(zip_item item);

// Dispose ZIP file data
extern void
zip_free_data(char *data);

// Cache ZIP file data
extern zip_cached
zip_cache(zip_item item);

// Reads from cached ZIP file data
extern void
zip_read(zip_cached handle, char *buff, off_t at, size_t size);

// Discard cached ZIP file data
extern void
zip_discard(zip_cached handle);

// Write file data to a file
bool
zip_extract_data(zip_item item, FILE *out);

// Get ZIP file size
extern uint32_t
zip_get_size(zip_item item);

// Calculate ZIP-compatible CRC-32 checksum of a buffer
// Returns checksum value
extern uint32_t
zip_calculate_crc(const uint8_t *buffer, size_t length);

// Calculate ZIP-compatible CRC-32 checksum of a data stream
// Returns checksum value
extern uint32_t
zip_calculate_crc_indirect(uint8_t (*stream)(void *, size_t),
                           void  *context,
                           size_t length);

#endif // _FMT_ZIP_H_

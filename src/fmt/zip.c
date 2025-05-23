#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <fmt/exe.h>
#include <fmt/utf8.h>
#include <fmt/zip.h>
#include <pal.h>

#ifndef CONFIG_COMPACT
static off_t _fbase = 0;
static int   _fd = -1;
static off_t _flen = 0;
#endif
static zip_cdir_file_header      *_cdir = NULL;
const static zip_cdir_end_header *_cden = NULL;

static int
_match_file_name(const char *name, uint16_t length, zip_cdir_file_header *cfh);

#ifndef CONFIG_COMPACT
static bool
_seek_read(void *ptr, off_t offset, size_t size)
{
    if (-1 == lseek(_fd, offset, SEEK_SET))
    {
        close(_fd);
        return false;
    }

    if (size != read(_fd, ptr, size))
    {
        close(_fd);
        return false;
    }

    return true;
}

static off_t
_get_bundle_length(void)
{
#ifdef _WIN32
    exe_dos_header         dos_header;
    exe_pe_optional_header optional_header = {0};
    exe_pe_data_directory  security = {0};

    ULONG  new_signature;
    char   padded_cdirend[sizeof(zip_cdir_end_header) + 7];
    size_t offset;

    if (!_seek_read(&dos_header, 0, sizeof(dos_header)))
    {
        return -1;
    }

    if (0x5A4D != dos_header.e_magic)
    {
        // Not an EXE
        return lseek(_fd, 0, SEEK_END);
    }

    if (!_seek_read(&new_signature, dos_header.e_lfanew, sizeof(new_signature)))
    {
        return -1;
    }

    if (0x00004550 != new_signature)
    {
        // Not a Portable Executable
        return lseek(_fd, 0, SEEK_END);
    }

    lseek(_fd, sizeof(exe_pe_file_header), SEEK_CUR);

    read(_fd, &optional_header, sizeof(optional_header));
    if (EXE_PE_DIRECTORY_ENTRY_SECURITY >= optional_header.NumberOfRvaAndSizes)
    {
        // No Security directory
        return lseek(_fd, 0, SEEK_END);
    }

    lseek(_fd, sizeof(exe_pe_data_directory) * EXE_PE_DIRECTORY_ENTRY_SECURITY,
          SEEK_CUR);
    read(_fd, &security, sizeof(security));
    if (0 == security.Size)
    {
        // Empty Security directory
        return lseek(_fd, 0, SEEK_END);
    }

    if (!_seek_read(padded_cdirend,
                    security.VirtualAddress - sizeof(padded_cdirend),
                    sizeof(padded_cdirend)))
    {
        return -1;
    }

    for (offset = 0; offset < 7; offset++)
    {
        zip_cdir_end_header *cdirend =
            (zip_cdir_end_header *)(padded_cdirend + offset);
        if ((ZIP_PK_SIGN == cdirend->pk_signature) &&
            (ZIP_CDIR_END_SIGN == cdirend->header_signature))
        {
            return (off_t)security.VirtualAddress - 7 + (off_t)offset;
        }
    }
#endif

    return lseek(_fd, 0, SEEK_END);
}
#endif

bool
zip_open(zip_archive archive)
{
    zip_cdir_end_header *cdirend;
#ifndef CONFIG_COMPACT
    zip_cdir_end_header cdirend_buff = {0};
    long                cdir_size;
#endif

    if (NULL != _cdir)
    {
        errno = EINVAL;
        return false;
    }

#ifdef CONFIG_COMPACT
    cdirend = archive;
#else
#ifdef O_BINARY
    _fd = open(archive, O_RDONLY | O_BINARY);
#else
    _fd = open(archive, O_RDONLY);
#endif
    if (-1 == _fd)
    {
        return false;
    }

    _flen = _get_bundle_length();
    if (-1 == _flen)
    {
        close(_fd);
        return false;
    }

    cdirend = &cdirend_buff;
    if (!_seek_read(cdirend, _flen - sizeof(zip_cdir_end_header),
                    sizeof(zip_cdir_end_header)))
    {
        return false;
    }
#endif

    if ((ZIP_PK_SIGN != cdirend->pk_signature) ||
        (ZIP_CDIR_END_SIGN != cdirend->header_signature))
    {
        errno = EFTYPE;
        return false;
    }

    if (cdirend->cdir_size > UINT16_MAX)
    {
        errno = EFBIG;
        return false;
    }

#ifdef CONFIG_COMPACT
    _cdir = (zip_cdir_file_header *)((char *)cdirend - cdirend->cdir_size);
#else
    cdir_size = cdirend->cdir_size + sizeof(zip_cdir_end_header);
    _cdir = (zip_cdir_file_header *)malloc(cdir_size);
    if (NULL == _cdir)
    {
        close(_fd);
        return false;
    }

    if (!_seek_read(_cdir, _flen - cdir_size, cdir_size))
    {
        free(_cdir);
        _cdir = NULL;
        return false;
    }
#endif

    _cden = (const zip_cdir_end_header *)((char *)_cdir + cdirend->cdir_size);
#ifndef CONFIG_COMPACT
    _fbase = _flen - (off_t)(_cden->cdir_offset + _cden->cdir_size +
                             (off_t)sizeof(zip_cdir_end_header));
#endif
    return true;
}

void
zip_close(void)
{
#ifndef CONFIG_COMPACT
    close(_fd);
#endif
}

int
zip_enum_files(zip_enum_files_callback callback, void *data)
{
    zip_cdir_file_header *cfh = _cdir;
    int                   i = 0;

    if (NULL == _cdir)
    {
        errno = EINVAL;
        return -1;
    }

    while (true)
    {
        if (ZIP_PK_SIGN != cfh->pk_signature)
        {
            errno = EFTYPE;
            return -1;
        }

        if (ZIP_CDIR_END_SIGN == cfh->header_signature)
        {
            return i;
        }

        if (ZIP_CDIR_FILE_SIGN != cfh->header_signature)
        {
            errno = EFTYPE;
            return -1;
        }

        if (!callback(cfh, data))
        {
            return i;
        }

        cfh = (zip_cdir_file_header *)((char *)cfh + cfh->name_length +
                                       cfh->extra_length + cfh->comment_length);
        cfh++;
        i++;
    }
}

typedef struct
{
    const char *name;
    uint16_t    length;

    zip_cdir_file_header *result;
} zip_search_context;

static bool
_zip_enum_files_callback(zip_cdir_file_header *cfh, void *data)
{
    zip_search_context *ctx = (zip_search_context *)data;

    int status = _match_file_name(ctx->name, ctx->length, cfh);
    if (0 == status)
    {
        ctx->result = cfh;
    }

    return 0 < status;
}

off_t
zip_search(const char *name, uint16_t length)
{
    zip_search_context ctx = {name, length, NULL};
    if (0 > zip_enum_files(_zip_enum_files_callback, &ctx))
    {
        return -1;
    }

    if (NULL != ctx.result)
    {
        return ctx.result->lfh_offset;
    }

    errno = ENOENT;
    return -1;
}

static off_t
_get_data(zip_item olfh, uint32_t *size, uint32_t *crc32)
{
    zip_local_file_header *lfh;
#ifndef CONFIG_COMPACT
    zip_local_file_header lfh_buff = {0};
#endif

    if (NULL == _cdir)
    {
        return -(errno = EINVAL);
    }

    if (0 > olfh)
    {
        return -(errno = EINVAL);
    }

#ifdef CONFIG_COMPACT
    char *base = (char *)_cdir - _cden->cdir_offset;
    lfh = (zip_local_file_header *)(base + olfh);
#else
    lfh = &lfh_buff;
    if (!_seek_read(lfh, _fbase + olfh, sizeof(zip_local_file_header)))
    {
        return -errno;
    }
#endif

    if ((ZIP_PK_SIGN != lfh->pk_signature) ||
        (ZIP_LOCAL_FILE_SIGN != lfh->header_signature))
    {
        return -(errno = EFTYPE);
    }
    if ((ZIP_METHOD_STORE != lfh->compression) ||
        (0 != (lfh->flags & ~ZIP_FLAGS_SUPPORTED)))
    {
        return -(errno = ENOSYS);
    }

    if (NULL != size)
    {
        *size = lfh->uncompressed_size;
    }

    if (NULL != crc32)
    {
        *crc32 = lfh->crc32;
    }

    return olfh + (off_t)sizeof(zip_local_file_header) +
           (off_t)lfh->name_length + (off_t)lfh->extra_length;
}

char *
zip_load_data(zip_item item)
{
    char    *buffer;
    uint32_t size = 0, crc32 = 0;
    off_t    odata = _get_data(item, &size, &crc32);

    if (0 > odata)
    {
        return NULL;
    }

#ifdef CONFIG_COMPACT
    char *base = (char *)_cdir - _cden->cdir_offset;
    buffer = base + odata;
#else
    buffer = (char *)malloc(size);
    if (NULL == buffer)
    {
        return NULL;
    }

    if (!_seek_read(buffer, _fbase + odata, size))
    {
        free(buffer);
        return NULL;
    }
#endif

    if (zip_calculate_crc((uint8_t *)buffer, size) != crc32)
    {
#ifndef CONFIG_COMPACT
        free(buffer);
#endif
        errno = EIO;
        return NULL;
    }

    return buffer;
}

void
zip_free_data(char *data)
{
#ifndef CONFIG_COMPACT
    free(data);
#endif
    return;
}

zip_cached
zip_cache(zip_item item)
{
    uint32_t size = 0, crc32 = 0;
    off_t    odata = _get_data(item, &size, &crc32);

#ifndef CONFIG_COMPACT
    return pal_cache(_fd, _fbase + odata, size);
#else
    return odata;
#endif
}

void
zip_read(zip_cached handle, char *buff, off_t at, size_t size)
{
#ifndef CONFIG_COMPACT
    pal_read(handle, buff, at, size);
#else
    char *base = (char *)_cdir - _cden->cdir_offset;
    memcpy(buff, base + handle + at, size);
#endif
}

void
zip_discard(zip_cached handle)
{
#ifndef CONFIG_COMPACT
    pal_discard(handle);
#endif
}

bool
zip_extract_data(zip_item item, FILE *out)
{
#ifndef CONFIG_COMPACT
    char buffer[512];
#endif
    uint32_t size = 0;
    off_t    odata = _get_data(item, &size, NULL);

    if (0 > odata)
    {
        return false;
    }

#ifdef CONFIG_COMPACT
    char *buffer = (char *)_cdir - _cden->cdir_offset + odata;
    return size == fwrite(buffer, 1, size, out);
#else
    while (512 < size)
    {
        if (!_seek_read(buffer, _fbase + odata, 512))
        {
            return false;
        }

        if (512 != fwrite(buffer, 1, 512, out))
        {
            return false;
        }

        odata += 512;
        size -= 512;
    }

    if (!_seek_read(buffer, _fbase + odata, size))
    {
        return false;
    }

    if (size != fwrite(buffer, 1, size, out))
    {
        return false;
    }

    return true;
#endif
}

uint32_t
zip_get_size(zip_item item)
{
    uint32_t size;
    _get_data(item, &size, NULL);
    return size;
}

// Check if ZIP Central Directory File Header matches provided name
// Returns 0 on match, 1 on no match, negative on error
int
_match_file_name(const char *name, uint16_t length, zip_cdir_file_header *cfh)
{
    zip_extra_fields_header *hdr, *end;
    uint16_t                 file_system;

    if (ZIP_PK_SIGN != cfh->pk_signature)
    {
        errno = EFTYPE;
        return -1;
    }

    if (ZIP_CDIR_END_SIGN == cfh->header_signature)
    {
        errno = ENOENT;
        return -1;
    }

    if (ZIP_CDIR_FILE_SIGN != cfh->header_signature)
    {
        errno = EFTYPE;
        return -1;
    }

    file_system = (cfh->version >> 8) & 0xFF;
    if ((ZIP_VERSION_FS_MSDOS != file_system) &&
        (ZIP_VERSION_FS_NTFS != file_system) &&
        (ZIP_VERSION_FS_VFAT != file_system))
    {
        // Regular comparison
        if (length != cfh->name_length)
        {
            return 1;
        }

        if (0 != memcmp(name, cfh->name, cfh->name_length))
        {
            return 1;
        }

        return 0;
    }

    // Case-insensitive comparison
    if ((length == cfh->name_length) &&
        (0 == utf8_strncasecmp(name, cfh->name, cfh->name_length)))
    {
        return 0;
    }

    hdr = (zip_extra_fields_header *)((char *)(cfh + 1) + cfh->name_length);
    end = (zip_extra_fields_header *)((char *)hdr + cfh->extra_length);
    while (hdr < end)
    {
        zip_extra_unicode_path_field *uni_name;
        uint16_t                      uni_name_length;

        if (ZIP_EXTRA_INFOZIP_UNICODE_PATH != hdr->signature)
        {
            hdr = (zip_extra_fields_header *)((char *)hdr + hdr->total_size);
            hdr++;
            continue;
        }

        uni_name = (zip_extra_unicode_path_field *)hdr;
        uni_name_length =
            uni_name->total_size - sizeof(zip_extra_unicode_path_field);
        if (0 ==
            utf8_strncasecmp(name, uni_name->unicode_name, uni_name_length))
        {
            return 0;
        }
    }

    return 1;
}

static const uint32_t _CRC_TABLE[] = {
    0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4,
    0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
    0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C};

#define _calculate_crc(byte_source)                                            \
    {                                                                          \
        size_t   i;                                                            \
        uint32_t crc = 0xFFFFFFFF;                                             \
        for (i = 0; i < length; i++)                                           \
        {                                                                      \
            uint8_t b = byte_source;                                           \
            crc = (crc >> 4) ^ _CRC_TABLE[(crc & 0xF) ^ (b & 0xF)];            \
            crc = (crc >> 4) ^ _CRC_TABLE[(crc & 0xF) ^ (b >> 4)];             \
        }                                                                      \
        return ~crc;                                                           \
    }

uint32_t
zip_calculate_crc(const uint8_t *buffer, size_t length)
{
    _calculate_crc(*buffer++);
}

uint32_t
zip_calculate_crc_indirect(uint8_t (*stream)(void *, size_t),
                           void  *context,
                           size_t length)
{
    _calculate_crc(stream(context, i));
}

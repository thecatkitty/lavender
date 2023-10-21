#ifdef __MINGW32__
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fmt/utf8.h>
#include <fmt/zip.h>

#ifndef ZIP_PIGGYBACK
static off_t _fbase = 0;
static int   _fd = -1;
static off_t _flen = 0;
#endif
static zip_cdir_file_header      *_cdir = NULL;
const static zip_cdir_end_header *_cden = NULL;

static int
_match_file_name(const char *name, uint16_t length, zip_cdir_file_header *cfh);

#ifndef ZIP_PIGGYBACK
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
#endif

bool
zip_open(zip_archive archive)
{
    if (NULL != _cdir)
    {
        errno = EINVAL;
        return false;
    }

    zip_cdir_end_header *cdirend;

#ifdef ZIP_PIGGYBACK
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

    _flen = lseek(_fd, 0, SEEK_END);
    if (-1 == _flen)
    {
        close(_fd);
        return false;
    }

    cdirend = alloca(sizeof(zip_cdir_end_header));
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

#ifdef ZIP_PIGGYBACK
    _cdir = (zip_cdir_file_header *)((void *)cdirend - cdirend->cdir_size);
#else
    long cdir_size = cdirend->cdir_size + sizeof(zip_cdir_end_header);
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

    _cden = (const zip_cdir_end_header *)((void *)_cdir + cdirend->cdir_size);
#ifndef ZIP_PIGGYBACK
    _fbase = _flen - _cden->cdir_offset - _cden->cdir_size -
             sizeof(zip_cdir_end_header);
#endif
    return true;
}

off_t
zip_search(const char *name, uint16_t length)
{
    if (NULL == _cdir)
    {
        errno = EINVAL;
        return -1;
    }

    zip_cdir_file_header *cfh = _cdir;
    while (true)
    {
        int status = _match_file_name(name, length, cfh);
        if (0 > status)
        {
            return -1;
        }

        if (0 == status)
        {
            return cfh->lfh_offset;
        }

        cfh = (zip_cdir_file_header *)((void *)cfh + cfh->name_length +
                                       cfh->extra_length + cfh->comment_length);
        cfh++;
    }

    return -1;
}

char *
zip_get_data(off_t olfh)
{
    if (NULL == _cdir)
    {
        errno = EINVAL;
        return NULL;
    }

    if (0 > olfh)
    {
        errno = EINVAL;
        return NULL;
    }

    zip_local_file_header *lfh;

#ifdef ZIP_PIGGYBACK
    void *base = (void *)_cdir - _cden->cdir_offset;
    lfh = (zip_local_file_header *)(base + olfh);
#else
    lfh = alloca(sizeof(zip_local_file_header));
    if (!_seek_read(lfh, _fbase + olfh, sizeof(zip_local_file_header)))
    {
        return false;
    }
#endif

    if ((ZIP_PK_SIGN != lfh->pk_signature) ||
        (ZIP_LOCAL_FILE_SIGN != lfh->header_signature))
    {
        errno = EFTYPE;
        return NULL;
    }

    if ((ZIP_METHOD_STORE != lfh->compression) ||
        (0 != (lfh->flags & ~ZIP_FLAGS_SUPPORTED)))
    {
        errno = ENOSYS;
        return NULL;
    }

    char *buffer;

#ifdef ZIP_PIGGYBACK
    buffer = (char *)(lfh + 1) + lfh->name_length + lfh->extra_length;
#else
    buffer = (char *)malloc(lfh->uncompressed_size);
    if (NULL == buffer)
    {
        return NULL;
    }

    if (!_seek_read(buffer,
                    _fbase + olfh + sizeof(zip_local_file_header) +
                        lfh->name_length + lfh->extra_length,
                    lfh->uncompressed_size))
    {
        free(buffer);
        return NULL;
    }
#endif

    if (zip_calculate_crc((uint8_t *)buffer, lfh->uncompressed_size) !=
        lfh->crc32)
    {
#ifndef ZIP_PIGGYBACK
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
#ifndef ZIP_PIGGYBACK
    free(data);
#endif
    return;
}

uint32_t
zip_get_size(off_t olfh)
{
    zip_local_file_header *lfh;

#ifdef ZIP_PIGGYBACK
    void *base = (void *)_cdir - _cden->cdir_offset;
    lfh = (zip_local_file_header *)(base + olfh);
#else
    off_t base = _flen - _cden->cdir_offset - _cden->cdir_size -
                 sizeof(zip_cdir_end_header);
    lfh = alloca(sizeof(zip_local_file_header));
    if (!_seek_read(lfh, base + olfh, sizeof(zip_local_file_header)))
    {
        return false;
    }
#endif

    return lfh->compressed_size;
}

// Check if ZIP Central Directory File Header matches provided name
// Returns 0 on match, 1 on no match, negative on error
int
_match_file_name(const char *name, uint16_t length, zip_cdir_file_header *cfh)
{
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

    uint16_t file_system = (cfh->version >> 8) & 0xFF;
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

    zip_extra_fields_header *hdr =
        (zip_extra_fields_header *)((void *)(cfh + 1) + cfh->name_length);
    zip_extra_fields_header *end =
        (zip_extra_fields_header *)((void *)hdr + cfh->extra_length);
    while (hdr < end)
    {
        if (ZIP_EXTRA_INFOZIP_UNICODE_PATH != hdr->signature)
        {
            hdr = (zip_extra_fields_header *)((void *)hdr + hdr->total_size);
            hdr++;
            continue;
        }

        zip_extra_unicode_path_field *uni_name =
            (zip_extra_unicode_path_field *)hdr;
        uint16_t uni_name_length =
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
    uint32_t crc = 0xFFFFFFFF;                                                 \
    for (int i = 0; i < length; i++)                                           \
    {                                                                          \
        uint8_t b = byte_source;                                               \
        crc = (crc >> 4) ^ _CRC_TABLE[(crc & 0xF) ^ (b & 0xF)];                \
        crc = (crc >> 4) ^ _CRC_TABLE[(crc & 0xF) ^ (b >> 4)];                 \
    }                                                                          \
    return ~crc;

uint32_t
zip_calculate_crc(uint8_t *buffer, size_t length)
{
    _calculate_crc(*buffer++);
}

uint32_t
zip_calculate_crc_indirect(uint8_t (*stream)(void *, size_t),
                           void   *context,
                           size_t  length)
{
    _calculate_crc(stream(context, i));
}

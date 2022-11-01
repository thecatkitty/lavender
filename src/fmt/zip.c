
#include <errno.h>
#include <string.h>

#include <fmt/utf8.h>
#include <fmt/zip.h>

#include <api/dos.h>

static zip_cdir_file_header      *_cdir = NULL;
const static zip_cdir_end_header *_cden = NULL;

static int
_match_file_name(const char *name, uint16_t length, zip_cdir_file_header *cfh);

bool
zip_open(zip_archive archive)
{
    if (NULL != _cdir)
    {
        errno = EINVAL;
        return false;
    }

    zip_cdir_end_header *cdirend;

    cdirend = archive;

    if (cdirend->cdir_size > UINT16_MAX)
    {
        errno = EFBIG;
        return false;
    }

    _cdir = (zip_cdir_file_header *)((void *)cdirend - cdirend->cdir_size);
    _cden = (const zip_cdir_end_header *)((void *)_cdir + cdirend->cdir_size);
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

    void *base = (void *)_cdir - _cden->cdir_offset;
    lfh = (zip_local_file_header *)(base + olfh);

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

    char *buffer = (char *)(lfh + 1) + lfh->name_length + lfh->extra_length;
    if (zip_calculate_crc((uint8_t *)buffer, lfh->uncompressed_size) !=
        lfh->crc32)
    {
        errno = EIO;
        return NULL;
    }

    return buffer;
}

void
zip_free_data(char *data)
{
    return;
}

uint32_t
zip_get_size(off_t olfh)
{
    zip_local_file_header *lfh;

    void *base = (void *)_cdir - _cden->cdir_offset;
    lfh = (zip_local_file_header *)(base + olfh);

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
zip_calculate_crc(uint8_t *buffer, int length)
{
    _calculate_crc(*buffer++);
}

uint32_t
zip_calculate_crc_indirect(uint8_t (*stream)(void *, size_t),
                           void *context,
                           int   length)
{
    _calculate_crc(stream(context, i));
}


#include <errno.h>
#include <string.h>

#include <cvt.h>
#include <fmt/zip.h>

static int
_match_file_name(const char *name, uint16_t length, ZIP_CDIR_FILE_HEADER *cfh);

ZIP_LOCAL_FILE_HEADER *
zip_search(ZIP_CDIR_END_HEADER *cdir, const char *name, uint16_t length)
{
    if (cdir->CentralDirectorySize > UINT16_MAX)
    {
        errno = EFBIG;
        return NULL;
    }

    ZIP_CDIR_FILE_HEADER *cfh =
        (ZIP_CDIR_FILE_HEADER *)((void *)cdir - cdir->CentralDirectorySize);
    ZIP_LOCAL_FILE_HEADER *lfh = NULL;
    while (!lfh)
    {
        int status = _match_file_name(name, length, cfh);
        if (0 > status)
        {
            return NULL;
        }

        if (0 == status)
        {
            void *base = (void *)cdir - cdir->CentralDirectoryOffset -
                         cdir->CentralDirectorySize;
            lfh = (ZIP_LOCAL_FILE_HEADER *)(base + cfh->LocalHeaderOffset);
        }

        cfh = (ZIP_CDIR_FILE_HEADER *)((void *)cfh + cfh->NameLength +
                                       cfh->ExtraLength + cfh->CommentLength);
        cfh++;
    }

    if ((ZIP_PK_SIGN != lfh->PkSignature) ||
        (ZIP_LOCAL_FILE_SIGN != lfh->HeaderSignature))
    {
        errno = EFTYPE;
        return NULL;
    }

    return lfh;
}

char *
zip_get_data(ZIP_LOCAL_FILE_HEADER *lfh, bool ignore_crc)
{
    if ((ZIP_PK_SIGN != lfh->PkSignature) ||
        (ZIP_LOCAL_FILE_SIGN != lfh->HeaderSignature))
    {
        errno = EFTYPE;
        return NULL;
    }

    if ((ZIP_METHOD_STORE != lfh->Compression) ||
        (0 != (lfh->Flags & ~ZIP_FLAGS_SUPPORTED)))
    {
        errno = ENOSYS;
        return NULL;
    }

    uint8_t *buffer = (uint8_t *)(lfh + 1) + lfh->NameLength + lfh->ExtraLength;
    if (!ignore_crc &&
        (zip_calculate_crc(buffer, lfh->UncompressedSize) != lfh->Crc32))
    {
        errno = EIO;
        return NULL;
    }

    return buffer;
}

// Check if ZIP Central Directory File Header matches provided name
// Returns 0 on match, 1 on no match, negative on error
int
_match_file_name(const char *name, uint16_t length, ZIP_CDIR_FILE_HEADER *cfh)
{
    if (ZIP_PK_SIGN != cfh->PkSignature)
    {
        errno = EFTYPE;
        return -1;
    }

    if (ZIP_CDIR_END_SIGN == cfh->HeaderSignature)
    {
        errno = ENOENT;
        return -1;
    }

    if (ZIP_CDIR_FILE_SIGN != cfh->HeaderSignature)
    {
        errno = EFTYPE;
        return -1;
    }

    uint16_t file_system = (cfh->Version >> 8) & 0xFF;
    if ((ZIP_VERSION_FS_MSDOS != file_system) &&
        (ZIP_VERSION_FS_NTFS != file_system) &&
        (ZIP_VERSION_FS_VFAT != file_system))
    {
        // Regular comparison
        if (length != cfh->NameLength)
        {
            return 1;
        }

        if (0 != memcmp(name, cfh->Name, cfh->NameLength))
        {
            return 1;
        }

        return 0;
    }

    // Case-insensitive comparison
    if ((length == cfh->NameLength) &&
        (0 == cvt_utf8_strncasecmp(name, cfh->Name, cfh->NameLength)))
    {
        return 0;
    }

    ZIP_EXTRA_FIELD_HEADER *hdr =
        (ZIP_EXTRA_FIELD_HEADER *)((void *)(cfh + 1) + cfh->NameLength);
    ZIP_EXTRA_FIELD_HEADER *end =
        (ZIP_EXTRA_FIELD_HEADER *)((void *)hdr + cfh->ExtraLength);
    while (hdr < end)
    {
        if (ZIP_EXTRA_INFOZIP_UNICODE_PATH != hdr->Signature)
        {
            hdr = (ZIP_EXTRA_FIELD_HEADER *)((void *)hdr + hdr->TotalSize);
            hdr++;
            continue;
        }

        ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD *uni_name =
            (ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD *)hdr;
        uint16_t uni_name_length =
            uni_name->TotalSize - sizeof(ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD);
        if (0 ==
            cvt_utf8_strncasecmp(name, uni_name->UnicodeName, uni_name_length))
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
zip_calculate_crc_indirect(uint8_t (*stream)(void *, int),
                           void *context,
                           int   length)
{
    _calculate_crc(stream(context, i));
}

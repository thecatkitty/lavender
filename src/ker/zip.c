
#include <string.h>

#include <ker.h>

static int
MatchFileName(const char *          name,
              uint16_t              nameLength,
              ZIP_CDIR_FILE_HEADER *cdirFile);

int
KerSearchArchive(ZIP_CDIR_END_HEADER *   cdir,
                 const char *            name,
                 uint16_t                nameLength,
                 ZIP_LOCAL_FILE_HEADER **lfh)
{
    if (cdir->CentralDirectorySize > UINT16_MAX)
    {
        ERR(KER_ARCHIVE_TOO_LARGE);
    }

    ZIP_CDIR_FILE_HEADER *cdirFile =
        (ZIP_CDIR_FILE_HEADER *)((void *)cdir - cdir->CentralDirectorySize);
    ZIP_LOCAL_FILE_HEADER *pLfh = NULL;
    while (!pLfh)
    {
        int status = MatchFileName(name, nameLength, cdirFile);
        if (0 > status)
            return status;

        if (0 == status)
        {
            void *base = (void *)cdir - cdir->CentralDirectoryOffset -
                         cdir->CentralDirectorySize;
            pLfh =
                (ZIP_LOCAL_FILE_HEADER *)(base + cdirFile->LocalHeaderOffset);
        }

        cdirFile =
            (ZIP_CDIR_FILE_HEADER *)((void *)cdirFile + cdirFile->NameLength +
                                     cdirFile->ExtraLength +
                                     cdirFile->CommentLength);
        cdirFile++;
    }

    if ((ZIP_PK_SIGN != pLfh->PkSignature) ||
        (ZIP_LOCAL_FILE_SIGN != pLfh->HeaderSignature))
    {
        ERR(KER_ARCHIVE_INVALID);
    }

    *lfh = pLfh;
    return 0;
}

int
KerGetArchiveData(ZIP_LOCAL_FILE_HEADER *lfh, void **data)
{
    if ((ZIP_PK_SIGN != lfh->PkSignature) ||
        (ZIP_LOCAL_FILE_SIGN != lfh->HeaderSignature))
    {
        ERR(KER_ARCHIVE_INVALID);
    }

    if ((ZIP_METHOD_STORE != lfh->Compression) ||
        (0 != (lfh->Flags & ~ZIP_FLAGS_SUPPORTED)))
    {
        ERR(KER_UNSUPPORTED);
    }

    uint8_t *buffer = (uint8_t *)(lfh + 1) + lfh->NameLength + lfh->ExtraLength;
    if (KerCalculateZipCrc(buffer, lfh->UncompressedSize) != lfh->Crc32)
    {
        *data = buffer;
        ERR(KER_INTEGRITY);
    }

    *data = buffer;
    return 0;
}

// Check if ZIP Central Directory File Header matches provided name
// Returns 0 on match, 1 on no match, negative on error
int
MatchFileName(const char *          name,
              uint16_t              nameLength,
              ZIP_CDIR_FILE_HEADER *cdirFile)
{
    if (ZIP_PK_SIGN != cdirFile->PkSignature)
    {
        ERR(KER_ARCHIVE_INVALID);
    }

    if (ZIP_CDIR_END_SIGN == cdirFile->HeaderSignature)
    {
        ERR(KER_NOT_FOUND);
    }

    if (ZIP_CDIR_FILE_SIGN != cdirFile->HeaderSignature)
    {
        ERR(KER_ARCHIVE_INVALID);
    }

    uint16_t fileSystem = (cdirFile->Version >> 8) & 0xFF;
    if ((ZIP_VERSION_FS_MSDOS != fileSystem) &&
        (ZIP_VERSION_FS_NTFS != fileSystem) &&
        (ZIP_VERSION_FS_VFAT != fileSystem))
    {
        // Regular comparison
        if (nameLength != cdirFile->NameLength)
        {
            return 1;
        }

        if (0 != memcmp(name, cdirFile->Name, cdirFile->NameLength))
        {
            return 1;
        }

        return 0;
    }

    // Case-insensitive comparison
    if ((nameLength == cdirFile->NameLength) &&
        (0 ==
         KerCompareUtf8IgnoreCase(name, cdirFile->Name, cdirFile->NameLength)))
    {
        return 0;
    }

    ZIP_EXTRA_FIELD_HEADER *hdr =
        (ZIP_EXTRA_FIELD_HEADER *)((void *)(cdirFile + 1) +
                                   cdirFile->NameLength);
    ZIP_EXTRA_FIELD_HEADER *end =
        (ZIP_EXTRA_FIELD_HEADER *)((void *)hdr + cdirFile->ExtraLength);
    while (hdr < end)
    {
        if (ZIP_EXTRA_INFOZIP_UNICODE_PATH != hdr->Signature)
        {
            hdr = (ZIP_EXTRA_FIELD_HEADER *)((void *)hdr + hdr->TotalSize);
            hdr++;
            continue;
        }

        ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD *uniName =
            (ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD *)hdr;
        uint16_t uniNameLength =
            uniName->TotalSize - sizeof(ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD);
        if (0 ==
            KerCompareUtf8IgnoreCase(name, uniName->UnicodeName, uniNameLength))
        {
            return 0;
        }
    }

    return 1;
}

static const uint32_t CRC_TABLE[] = {
    0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4,
    0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
    0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C};

uint32_t
KerCalculateZipCrc(uint8_t *buffer, int length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < length; i++)
    {
        uint8_t b = *buffer++;
        crc = (crc >> 4) ^ CRC_TABLE[(crc & 0xF) ^ (b & 0xF)];
        crc = (crc >> 4) ^ CRC_TABLE[(crc & 0xF) ^ (b >> 4)];
    }
    return ~crc;
}

uint32_t
KerCalculateZipCrcIndirect(uint8_t (*stream)(void *, int),
                           void *context,
                           int   length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < length; i++)
    {
        uint8_t b = stream(context, i);
        crc = (crc >> 4) ^ CRC_TABLE[(crc & 0xF) ^ (b & 0xF)];
        crc = (crc >> 4) ^ CRC_TABLE[(crc & 0xF) ^ (b >> 4)];
    }
    return ~crc;
}

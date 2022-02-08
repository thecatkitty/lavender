#include <fcntl.h>
#include <libi86/string.h>
#include <unistd.h>

#include <api/bios.h>
#include <api/dos.h>
#include <fmt/exe.h>
#include <fmt/fat.h>
#include <ker.h>

extern DOS_PSP *KerPsp;

static void
CopyVolumeLabel(char *dst, const char *src);

bool
KerIsDosBox(void)
{
    const char far *str = (const char far *)0xF000E061;
    for (const char *i = "DOSBox"; *i; i++, str++)
    {
        if (*i != *str)
            return false;
    }
    return true;
}

bool
KerIsDosMajor(uint8_t major)
{
    uint8_t dosMajor = DosGetVersion() & 0xFF;
    return (1 == major) ? (0 == dosMajor) : (major == dosMajor);
}

bool
KerIsWindowsNt(void)
{
    // NTVDM claims to be DOS 5
    if (!KerIsDosMajor(5))
    {
        return false; // It's not 5, so it's not NTVDM.
    }

    // Does it have %OS% environment variable?
    far const char *os = KerGetEnvironmentVariable("OS");
    if (0 == FP_SEG(os))
    {
        return false; // It's just a regular DOS 5.
    }

    // Does it claim to be Windows NT?
    const char windowsNt[] = "Windows_NT";
    return 0 == _fmemcmp(os, windowsNt, sizeof(windowsNt));
}

uint16_t
KerGetWindowsNtVersion(void)
{
    far const char *systemRoot = KerGetEnvironmentVariable("SYSTEMROOT");
    if (NULL == systemRoot)
    {
        return 0; // No system root
    }

    char smssPath[261];
    _fmemcpy(smssPath, systemRoot, _fstrlen(systemRoot) + 1);
    strcat(smssPath, "\\system32\\smss.exe");

    int fd;
    if (0 > (fd = open(smssPath, O_RDONLY)))
    {
        return 0; // No SMSS
    }

    union {
        EXE_DOS_HEADER         DosHdr;
        ULONG                  NewSignature;
        EXE_PE_OPTIONAL_HEADER OptionalHeader;
    } data;

    read(fd, &data, sizeof(EXE_DOS_HEADER));
    if (0x5A4D != data.DosHdr.e_magic)
    {
        return 0; // Invalid executable
    }

    lseek(fd, data.DosHdr.e_lfanew, SEEK_SET);
    read(fd, &data, sizeof(ULONG));
    if (0x00004550 != data.NewSignature)
    {
        return 0; // Invalid executable
    }

    lseek(fd, sizeof(EXE_PE_FILE_HEADER), SEEK_CUR);
    read(fd, &data, sizeof(EXE_PE_OPTIONAL_HEADER));
    close(fd);

    return (data.OptionalHeader.MajorImageVersion << 8) |
           data.OptionalHeader.MinorImageVersion;
}

far const char *
KerGetEnvironmentVariable(const char *key)
{
    far const char *env = MK_FP(KerPsp->EnvironmentSegment, 0);
    size_t          keyLength = strlen(key);

    while (*env)
    {
        if ((0 == _fmemcmp(key, env, keyLength)) && ('=' == env[keyLength]))
        {
            return env + keyLength + 1;
        }

        env += _fstrlen(env) + 1;
    }

    return MK_FP(0, 0);
}

int
KerGetFloppyDriveCount(void)
{
    BIOS_EQUIPMENT equipment;
    *(short *)&equipment = BiosGetEquipmentList();
    return equipment.FloppyDisk ? (equipment.FloppyDrives + 1) : 0;
}

int
KerGetVolumeInfo(uint8_t drive, KER_VOLUME_INFO *out)
{
    union {
        char                Bytes[512];
        BOOT_SECTOR         Boot;
        FAT_DIRECTORY_ENTRY Root[512 / sizeof(FAT_DIRECTORY_ENTRY)];
    } sector;

    if (0 != DosReadDiskAbsolute(drive, 1, 0, sector.Bytes))
    {
        ERR(KER_DISK_ACCESS);
    }

    if (0xAA55U != sector.Boot.Magic)
    {
        ERR(KER_UNSUPPORTED);
    }

    int offset, size;
    switch ((uint8_t)sector.Boot.Jump[0])
    {
    case 0xEB: // JMP rel8
        offset = *(int8_t *)(sector.Boot.Jump + 1);
        size = offset - (sizeof(sector.Boot.OemString) + 1);
        break;
    case 0xE9: // JMP rel16
        offset = *(int16_t *)(sector.Boot.Jump + 1);
        size = offset - sizeof(sector.Boot.OemString);
        break;
    default:
        ERR(KER_UNSUPPORTED);
    }

    switch (size)
    {
    case sizeof(BPB_DOS20):
    case sizeof(BPB_DOS30):
    case sizeof(BPB_DOS32):
    case sizeof(BPB_DOS33):
        out->SerialNumber = 0;
        out->Label[0] = 0;
        break;
    case sizeof(BPB_DOS34): {
        BPB_DOS34 *bpb = (BPB_DOS34 *)sector.Boot.Payload;
        out->SerialNumber = bpb->SerialNumber;
        out->Label[0] = 0;
        break;
    }
    case sizeof(BPB_DOS40): {
        BPB_DOS40 *bpb = (BPB_DOS40 *)sector.Boot.Payload;
        out->SerialNumber = bpb->Bpb34.SerialNumber;
        CopyVolumeLabel(out->Label, bpb->Label);
        break;
    }
    case sizeof(BPB_DOS71): {
        BPB_DOS71 *bpb = (BPB_DOS71 *)sector.Boot.Payload;
        out->SerialNumber = bpb->SerialNumber;
        out->Label[0] = 0;
        break;
    }
    case sizeof(BPB_DOS71_FULL): {
        BPB_DOS71_FULL *bpb = (BPB_DOS71_FULL *)sector.Boot.Payload;
        out->SerialNumber = bpb->Bpb71.SerialNumber;
        CopyVolumeLabel(out->Label, bpb->Label);
        break;
    }
    default:
        ERR(KER_UNSUPPORTED);
    }

    BPB_DOS20 *bpb = (BPB_DOS20 *)sector.Boot.Payload;
    uint16_t   rootDirEntries = bpb->RootEntries;
    uint16_t rootDirSectors = ((rootDirEntries * sizeof(FAT_DIRECTORY_ENTRY)) +
                               (bpb->BytesPerSector - 1)) /
                              bpb->BytesPerSector;
    uint16_t firstDataSector = bpb->ReservedSectors +
                               (bpb->Fats * bpb->SectorsPerFat) +
                               rootDirSectors;
    uint16_t firstRootDirSector = firstDataSector - rootDirSectors;

    if (0 != DosReadDiskAbsolute(drive, 1, firstRootDirSector, sector.Bytes))
    {
        return 0;
    }

    for (int i = 0; i < (sizeof(sector) / sizeof(FAT_DIRECTORY_ENTRY)); i++)
    {
        if (FAT_ATTRIBUTE_VOLUME_ID != sector.Root[i].Attributes)
            continue;

        CopyVolumeLabel(out->Label, sector.Root[i].FileName);
        break;
    }
    return 0;
}

void
CopyVolumeLabel(char *dst, const char *src)
{
    memcpy(dst, src, 11);

    char *last = dst + 11;
    last[0] = ' ';
    while (' ' == *last)
        last--;
    last[1] = 0;
}

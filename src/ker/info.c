#include <string.h>

#include <api/bios.h>
#include <api/dos.h>
#include <fmt/fat.h>
#include <ker.h>

static void
CopyVolumeLabel(char *dst, const char *src);

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

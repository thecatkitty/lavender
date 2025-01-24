#include <arch/dos/bios.h>
#include <fmt/fat.h>
#include <pal.h>

static int
read_sector(uint8_t  drive,
            uint16_t cylinder,
            uint8_t  head,
            uint8_t  sector,
            char    *buffer)
{
    short status;
    for (int i = 0; i < 3; ++i)
    {
        status = bios_read_sectors(drive, cylinder, head, sector, 1, buffer);
        if (0 == (status & 0xFF00))
        {
            return 0;
        }

        bios_reset_disk(drive);
    }

    return -((status >> 8) & 0xFF);
}

static void
copy_label(char *dst, const char *src)
{
    memcpy(dst, src, 11);

    char *last = dst + 11;
    last[0] = ' ';
    while (' ' == *last)
        last--;
    last[1] = 0;
}

typedef struct
{
    char     label[12];
    uint32_t serial_number;
} volume_info;

static bool
copy_info(uint8_t drive, volume_info *out)
{
    union {
        char                bytes[512];
        fat_boot_sector     boot;
        fat_directory_entry root[512 / sizeof(fat_directory_entry)];
    } sector;

    if (0 > read_sector(drive, 0, 0, 1, sector.bytes))
    {
        return false;
    }

    int offset, size;
    switch ((uint8_t)sector.boot.Ia32Jump[0])
    {
    case 0xEB: // JMP rel8
        offset = *(int8_t *)(sector.boot.Ia32Jump + 1);
        size = offset - (sizeof(sector.boot.OemId) + 1);
        break;
    case 0xE9: // JMP rel16
        offset = *(int16_t *)(sector.boot.Ia32Jump + 1);
        size = offset - sizeof(sector.boot.OemId);
        break;
    default:
        return false;
    }

    switch (size)
    {
    case sizeof(fat_bpb20):
    case sizeof(fat_bpb30):
    case sizeof(fat_bpb32):
    case sizeof(fat_bpb33):
        out->serial_number = 0;
        out->label[0] = 0;
        break;
    case sizeof(fat_bpb34): {
        fat_bpb34 *bpb = (fat_bpb34 *)sector.boot.Payload;
        out->serial_number = bpb->Id;
        out->label[0] = 0;
        break;
    }
    case sizeof(fat_bpb40):
    case sizeof(fat_bpb71):
    case sizeof(fat_bpb71_full): {
        fat_bpb40 *bpb = (fat_bpb40 *)sector.boot.Payload;
        out->serial_number = bpb->bpb34.Id;
        copy_label(out->label, bpb->FatLabel);
        break;
    }
    default:
        return false;
    }

    fat_bpb20 *bpb = (fat_bpb20 *)sector.boot.Payload;
    uint16_t   root_entries = bpb->RootEntries;
    uint16_t   root_sectors =
        ((root_entries * sizeof(fat_directory_entry)) + (bpb->SectorSize - 1)) /
        bpb->SectorSize;
    uint16_t first_data_sector = bpb->ReservedSectors +
                                 (bpb->NoFats * bpb->SectorsPerFat) +
                                 root_sectors;
    uint16_t first_root_sector = first_data_sector - root_sectors;
    if (63 <= first_root_sector)
    {
        return false;
    }

    if (0 > read_sector(drive, 0, 0, 1 + first_root_sector, sector.bytes))
    {
        return false;
    }

    for (int i = 0; i < (sizeof(sector) / sizeof(fat_directory_entry)); i++)
    {
        if (FAT_ATTRIBUTE_VOLUME_ID != sector.root[i].Attributes)
            continue;

        copy_label(out->label, sector.root[i].FileName);
        return true;
    }

    return false;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    union {
        short          w;
        bios_equipment s;
    } equipment;
    equipment.w = bios_get_equipment_list();

    int drives = equipment.s.floppy_disk ? (equipment.s.floppy_drives + 1) : 0;
    if (2 < drives)
    {
        drives = 2;
    }

    volume_info volume;

    uint8_t drive;
    for (drive = 0; drive < drives; drive++)
    {
        if (!copy_info(drive, &volume))
            continue;

        if (0 == strcmp(volume.label, tag))
            return volume.serial_number;
    }

    errno = ENOENT;
    return 0;
}

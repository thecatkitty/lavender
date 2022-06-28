#ifndef _FMT_FAT_H_
#define _FMT_FAT_H_

#include <assert.h>

#include <base.h>

#define FAT_ATTRIBUTE_READ_ONLY 0x01
#define FAT_ATTRIBUTE_HIDDEN    0x02
#define FAT_ATTRIBUTE_SYSTEM    0x04
#define FAT_ATTRIBUTE_VOLUME_ID 0x08
#define FAT_ATTRIBUTE_DIRECTORY 0x10
#define FAT_ATTRIBUTE_ARCHIVE   0x20
#define FAT_ATTRIBUTE_LFN                                                      \
    (FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM |   \
     FAT_ATTRIBUTE_VOLUME_ID)

#pragma pack(push, 1)

typedef struct
{
    char     Ia32Jump[3];
    char     OemId[8];
    char     Payload[512 - (3 + 8 + sizeof(uint16_t))];
    uint16_t Magic;
} fat_boot_sector;

static_assert(512 == sizeof(fat_boot_sector),
              "Boot sector size doesn't match specification");

typedef struct
{
    uint16_t SectorSize;
    uint8_t  SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t  NoFats;
    uint16_t RootEntries;
    uint16_t Sectors;
    uint8_t  Media;
    uint16_t SectorsPerFat;
} fat_bpb20;

static_assert(13 == sizeof(fat_bpb20),
              "DOS 2.0 BPB size doesn't match specification");

typedef struct
{
    fat_bpb20 bpb20;
    uint16_t  SectorsPerTrack;
    uint16_t  Heads;
    uint16_t  HiddenSectors;
} fat_bpb30;

static_assert(19 == sizeof(fat_bpb30),
              "DOS 3.0 BPB size doesn't match specification");

typedef struct
{
    fat_bpb30 bpb30;
    uint16_t  Sectors;
} fat_bpb32;

static_assert(21 == sizeof(fat_bpb32),
              "DOS 3.2 BPB size doesn't match specification");

typedef struct
{
    fat_bpb20 bpb20;
    uint16_t  SectorsPerTrack;
    uint16_t  Heads;
    uint32_t  HiddenSectors;
    uint32_t  LargeSectors;
} fat_bpb33;

static_assert(25 == sizeof(fat_bpb33),
              "DOS 3.3 BPB size doesn't match specification");

typedef struct
{
    fat_bpb33 bpb33;
    uint8_t   PhysicalDriveNumber;
    uint8_t   CurrentHead;
    uint8_t   Signature;
    uint32_t  Id;
} fat_bpb34;

static_assert(32 == sizeof(fat_bpb34),
              "DOS 3.4 BPB size doesn't match specification");

typedef struct
{
    fat_bpb34 bpb34;
    char      FatLabel[11];
    char      SystemId[8];
} fat_bpb40;

static_assert(51 == sizeof(fat_bpb40),
              "DOS 4.0 BPB size doesn't match specification");

typedef struct
{
    fat_bpb33 bpb33;
    uint32_t  LargeSectorsPerFat;
    uint16_t  ExtendedFlags;
    uint16_t  FsVersion;
    uint32_t  RootDirFirstCluster;
    uint16_t  FsInfoSector;
    uint16_t  BackupBootSector;
    char      Reserved[12];
    uint8_t   PhysicalDriveNumber;
    uint8_t   CurrentHead;
    uint8_t   Signature;
    uint32_t  Id;
} fat_bpb71;

static_assert(60 == sizeof(fat_bpb71),
              "DOS 7.1 short BPB size doesn't match specification");

typedef struct
{
    fat_bpb71 bpb71;
    char      FatLabel[11];
    char      SystemId[8];
} fat_bpb71_full;

static_assert(79 == sizeof(fat_bpb71_full),
              "DOS 7.1 full BPB size doesn't match specification");

typedef struct
{
    unsigned DoubleSecond : 5;
    unsigned Minute : 6;
    unsigned Hour : 5;
} fat_time;

static_assert(sizeof(uint16_t) == sizeof(fat_time),
              "FAT time struct size doesn't match specification");

typedef struct
{
    unsigned Day : 5;
    unsigned Month : 4;
    unsigned Year : 7;
} fat_date;

static_assert(sizeof(uint16_t) == sizeof(fat_date),
              "FAT date struct size doesn't match specification");

typedef struct
{
    fat_time Time;
    fat_date Date;
} fat_date_time;

static_assert(sizeof(uint32_t) == sizeof(fat_date_time),
              "FAT date and time struct size doesn't match specification");

typedef struct
{
    char          FileName[11];
    uint8_t       Attributes;
    uint8_t       CaseFlag;
    uint8_t       CreateMillisecond;
    fat_date_time FileCreateTime;
    fat_date      FileLastAccess;
    uint16_t      FileClusterHigh;
    fat_date_time FileModificationTime;
    uint16_t      FileCluster;
    uint32_t      FileSize;
} fat_directory_entry;

static_assert(32 == sizeof(fat_directory_entry),
              "FAT directory entry size doesn't match specification");

#pragma pack(pop)

#endif // _FMT_FAT_H_

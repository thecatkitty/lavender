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
    char     Jump[3];
    char     OemString[8];
    char     Payload[512 - (3 + 8 + sizeof(uint16_t))];
    uint16_t Magic;
} BOOT_SECTOR;

static_assert(512 == sizeof(BOOT_SECTOR),
              "Boot sector size doesn't match specification");

typedef struct
{
    uint16_t BytesPerSector;
    uint8_t  SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t  Fats;
    uint16_t RootEntries;
    uint16_t Sectors;
    uint8_t  MediaDescriptor;
    uint16_t SectorsPerFat;
} BPB_DOS20;

static_assert(13 == sizeof(BPB_DOS20),
              "DOS 2.0 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS20 Bpb20;
    uint16_t  SectorsPerTrack;
    uint16_t  Heads;
    uint16_t  HiddenSectors;
} BPB_DOS30;

static_assert(19 == sizeof(BPB_DOS30),
              "DOS 3.0 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS30 Bpb30;
    uint16_t  Sectors;
} BPB_DOS32;

static_assert(21 == sizeof(BPB_DOS32),
              "DOS 3.2 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS20 Bpb20;
    uint16_t  SectorsPerTrack;
    uint16_t  Heads;
    uint32_t  HiddenSectors;
    uint32_t  Sectors;
} BPB_DOS33;

static_assert(25 == sizeof(BPB_DOS33),
              "DOS 3.3 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS33 Bpb33;
    uint8_t   DriveNumber;
    uint8_t   Flags;
    uint8_t   EbSignature;
    uint32_t  SerialNumber;
} BPB_DOS34;

static_assert(32 == sizeof(BPB_DOS34),
              "DOS 3.4 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS34 Bpb34;
    char      Label[11];
    char      FsType[8];
} BPB_DOS40;

static_assert(51 == sizeof(BPB_DOS40),
              "DOS 4.0 BPB size doesn't match specification");

typedef struct
{
    BPB_DOS33 Bpb33;
    uint32_t  SectorsPerFat;
    uint16_t  Flags1;
    uint16_t  Version;
    uint32_t  RootCluster;
    uint16_t  FsInfoSector;
    uint16_t  BackupSector;
    char      Reserved[12];
    uint8_t   DriveNumber;
    uint8_t   Flags2;
    uint8_t   EbSignature;
    uint32_t  SerialNumber;
} BPB_DOS71;

static_assert(60 == sizeof(BPB_DOS71),
              "DOS 7.1 short BPB size doesn't match specification");

typedef struct
{
    BPB_DOS71 Bpb71;
    char      Label[11];
    char      FsType[8];
} BPB_DOS71_FULL;

static_assert(79 == sizeof(BPB_DOS71_FULL),
              "DOS 7.1 full BPB size doesn't match specification");

typedef struct
{
    unsigned DoubleSecond : 5;
    unsigned Minute : 6;
    unsigned Hour : 5;
} FAT_TIME;

static_assert(sizeof(uint16_t) == sizeof(FAT_TIME),
              "FAT time struct size doesn't match specification");

typedef struct
{
    unsigned Day : 5;
    unsigned Month : 4;
    unsigned Year : 7;
} FAT_DATE;

static_assert(sizeof(uint16_t) == sizeof(FAT_DATE),
              "FAT date struct size doesn't match specification");

typedef struct
{
    char     FileName[11];
    uint8_t  Attributes;
    uint8_t  Reserved;
    uint8_t  CreationCentiseconds;
    FAT_TIME CreationTime;
    FAT_DATE CreationDate;
    FAT_DATE AccessDate;
    uint16_t FirstClusterHigh;
    FAT_TIME ModificationTime;
    FAT_DATE ModificationDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} FAT_DIRECTORY_ENTRY;

static_assert(32 == sizeof(FAT_DIRECTORY_ENTRY),
              "FAT directory entry size doesn't match specification");

#pragma pack(pop)

#endif // _FMT_FAT_H_

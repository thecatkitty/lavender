#ifndef _FMT_EXE_H_
#define _FMT_EXE_H_

#include <inttypes.h>

#ifndef VOID
typedef uint8_t  UCHAR;
typedef uint16_t USHORT;
typedef int32_t  LONG;
typedef uint32_t ULONG;

typedef uint16_t WORD;
typedef uint32_t DWORD;
#endif

#pragma pack(push, 1)
typedef struct
{                      // DOS .EXE header
    USHORT e_magic;    // Magic number
    USHORT e_cblp;     // Bytes on last page of file
    USHORT e_cp;       // Pages in file
    USHORT e_crlc;     // Relocations
    USHORT e_cparhdr;  // Size of header in paragraphs
    USHORT e_minalloc; // Minimum extra paragraphs needed
    USHORT e_maxalloc; // Maximum extra paragraphs needed
    USHORT e_ss;       // Initial (relative) SS value
    USHORT e_sp;       // Initial SP value
    USHORT e_csum;     // Checksum
    USHORT e_ip;       // Initial IP value
    USHORT e_cs;       // Initial (relative) CS value
    USHORT e_lfarlc;   // File address of relocation table
    USHORT e_ovno;     // Overlay number
    USHORT e_res[4];   // Reserved words
    USHORT e_oemid;    // OEM identifier (for e_oeminfo)
    USHORT e_oeminfo;  // OEM information; e_oemid specific
    USHORT e_res2[10]; // Reserved words
    LONG   e_lfanew;   // File address of new exe header
} exe_dos_header;

typedef struct
{
    USHORT Machine;
    USHORT NumberOfSections;
    ULONG  TimeDateStamp;
    ULONG  PointerToSymbolTable;
    ULONG  NumberOfSymbols;
    USHORT SizeOfOptionalHeader;
    USHORT Characteristics;
} exe_pe_file_header;

typedef struct
{
    //
    // Standard fields.
    //
    USHORT Magic;
    UCHAR  MajorLinkerVersion;
    UCHAR  MinorLinkerVersion;
    ULONG  SizeOfCode;
    ULONG  SizeOfInitializedData;
    ULONG  SizeOfUninitializedData;
    ULONG  AddressOfEntryPoint;
    ULONG  BaseOfCode;
    ULONG  BaseOfData;
    //
    // NT additional fields.
    //
    ULONG  ImageBase;
    ULONG  SectionAlignment;
    ULONG  FileAlignment;
    USHORT MajorOperatingSystemVersion;
    USHORT MinorOperatingSystemVersion;
    USHORT MajorImageVersion;
    USHORT MinorImageVersion;
    USHORT MajorSubsystemVersion;
    USHORT MinorSubsystemVersion;
    ULONG  Reserved1;
    ULONG  SizeOfImage;
    ULONG  SizeOfHeaders;
    ULONG  CheckSum;
    USHORT Subsystem;
    USHORT DllCharacteristics;
    ULONG  SizeOfStackReserve;
    ULONG  SizeOfStackCommit;
    ULONG  SizeOfHeapReserve;
    ULONG  SizeOfHeapCommit;
    ULONG  LoaderFlags;
    ULONG  NumberOfRvaAndSizes;
    //    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} exe_pe_optional_header;

typedef struct
{
    DWORD VirtualAddress;
    DWORD Size;
} exe_pe_data_directory;

#pragma pack(pop)

#define EXE_PE_DIRECTORY_ENTRY_SECURITY 4

#endif // _FMT_EXE_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#if defined(_MSC_VER)
#include <intrin.h>

#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP16(x) __builtin_bswap16(x)
#endif

#include <ard/version.h>

typedef struct
{
    WORD        version;
    const char *name;
} os_version;

static const os_version WIN_VERSIONS[] = {
    {0x0400, "95"}, {0x040A, "98"}, {0x045A, "Me"}, {0xFFFF, NULL}};

static const os_version WINNT_VERSIONS[] = {
    {0x030A, "NT 3.1"},      {0x0332, "NT 3.5"}, {0x0333, "NT 3.51"},
    {0x0400, "NT 4.0"},      {0x0500, "2000"},   {0x0501, "XP"},
    {0x0502, "Server 2003"}, {0x0600, "Vista"},  {0x0601, "7"},
    {0x0602, "8"},           {0x0603, "8.1"},    {0x0604, "Technical Preview"},
    {0x0A00, "10"},          {0xFFFF, NULL}};

bool
ardv_windows_is_nt(void)
{
    return 0 == (GetVersion() & 0x80000000);
}

WORD
ardv_windows_get_version(void)
{
    return BSWAP16(LOWORD(GetVersion()));
}

static const char *
find_name(_In_ const os_version *names, _In_ WORD version)
{
    while (version > names->version)
    {
        names++;
    }

    return names->name;
}

const char *
ardv_windows_get_name(_In_ WORD version, _In_ bool is_nt)
{
    return find_name(is_nt ? WINNT_VERSIONS : WIN_VERSIONS, version);
}

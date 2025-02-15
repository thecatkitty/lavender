#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>

#if defined(_MSC_VER)
#include <intrin.h>

#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP16(x) __builtin_bswap16(x)
#endif

#include <ard/version.h>

typedef BOOL(WINAPI *pf_getversionex)(LPOSVERSIONINFO);

typedef struct
{
    DWORD       version;
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

static const os_version OSR_VERSIONS[] = {{0x04000200, "OSR 2"},
                                          {0x04000205, "OSR 2.5"},
                                          {0x040A0100, "SE"},
                                          {0xFFFFFFFF, NULL}};

static char sp_name_[128];

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

WORD
ardv_windows_get_servicepack(void)
{
    OSVERSIONINFOEX osvi;
    pf_getversionex fn = NULL;
    HMODULE         kernel32 = GetModuleHandle("kernel32.dll");
    if (NULL == kernel32)
    {
        // can't tell
        return 0;
    }

    fn = (pf_getversionex)GetProcAddress(kernel32, "GetVersionExA");
    if (NULL == fn)
    {
        // Windows NT 3.1 or Win32s
        return 0;
    }

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (fn((LPOSVERSIONINFO)&osvi))
    {
        // Windows NT 4.0 SP6 or later
        return (osvi.wServicePackMajor << 8) | osvi.wServicePackMinor;
    }

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!fn((LPOSVERSIONINFO)&osvi))
    {
        // can't tell
        return 0;
    }

    if (0 == osvi.szCSDVersion[0])
    {
        // no service pack
        return 0;
    }

    if (' ' == osvi.szCSDVersion[0])
    {
        // Windows 95 or 98
        switch (osvi.szCSDVersion[1])
        {
        case 'A':
            return 0x0100; // Windows 98 SE
        case 'B':
            return 0x0200; // Windows 95 OSR 2
        case 'C':
            return 0x0205; // Windows 95 OSR 2.5
        }
        return 0;
    }

    if ('S' == osvi.szCSDVersion[0])
    {
        // some service pack present
        return (osvi.szCSDVersion[13] - '0') << 8;
    }
    // can't tell
    return 0;
}

static const char *
find_name(_In_ const os_version *names, _In_ DWORD version)
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

const char *
ardv_windows_get_spname(_In_ WORD os_version,
                        _In_ WORD sp_version,
                        _In_ bool is_nt)
{
    if (is_nt)
    {
        int length = sprintf(sp_name_, "Service Pack %u", sp_version >> 8);
        if (sp_version & 0xFF)
        {
            sp_name_[length++] = 'a' + (sp_version & 0xFF) - 1;
            sp_name_[length] = 0;
        }

        return sp_name_;
    }

    return find_name(OSR_VERSIONS, ((DWORD)os_version << 16) | sp_version);
}

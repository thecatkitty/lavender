#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define UNICODE
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <shlobj.h>
#include <windows.h>

#include <fmt/utf8.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <snd.h>

#include "../resource.h"
#include "pal_impl.h"

static LPVOID _ver_resource = NULL;
static WORD  *_ver_vfi_translation = NULL;
static char   _ver_string[MAX_PATH] = {0};

extern SDL_Window *_window;

static HWND          _wnd = NULL;
static char         *_font = NULL;
static LARGE_INTEGER _start_pc, _pc_freq;

void
pal_initialize(int argc, char *argv[])
{
    QueryPerformanceFrequency(&_pc_freq);
    QueryPerformanceCounter(&_start_pc);

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");

        WCHAR msg[MAX_PATH];
        LoadStringW(NULL, IDS_NOARCHIVE, msg, MAX_PATH);
        MessageBoxW(NULL, msg, L"Lavender", MB_ICONERROR);
        exit(1);
    }

    if (!sdl2arch_initialize())
    {
        LOG("SDL2 architecture initialization failed");

        WCHAR msg[MAX_PATH];
        LoadStringW(NULL, IDS_UNSUPPENV, msg, MAX_PATH);
        MessageBoxW(NULL, msg, L"Lavender", MB_ICONERROR);
        exit(1);
    }

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(_window, &wminfo);
    _wnd = wminfo.info.win.window;

    if (!snd_initialize())
    {
        LOG("cannot initialize sound");
    }
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (_font)
    {
        free(_font);
    }

    sdl2arch_cleanup();
    ziparch_cleanup();
}

uint32_t
pal_get_counter(void)
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return (pc.QuadPart - _start_pc.QuadPart) / (_pc_freq.QuadPart / 1000);
}

uint32_t
pal_get_ticks(unsigned ms)
{
    return ms;
}

void
pal_sleep(unsigned ms)
{
    LOG("entry, ms: %u", ms);

    Sleep(ms);
}

static bool
_load_version_info(void)
{
    HRSRC resource_info = FindResourceW(NULL, MAKEINTRESOURCEW(1), RT_VERSION);
    if (NULL == resource_info)
    {
        LOG("cannot find version resource");
        return false;
    }

    DWORD   resource_size = SizeofResource(NULL, resource_info);
    HGLOBAL resource_data = LoadResource(NULL, resource_info);
    if (NULL == resource_data)
    {
        LOG("cannot load version resource");
        return false;
    }

    LPVOID resource = LockResource(resource_data);
    if (NULL == resource)
    {
        LOG("cannot lock version resource");
        FreeResource(resource_data);
        return false;
    }

    _ver_resource = LocalAlloc(LMEM_FIXED, resource_size);
    if (NULL == _ver_resource)
    {
        LOG("cannot allocate memory for version resource");
        FreeResource(resource_data);
        return false;
    }

    CopyMemory(_ver_resource, resource, resource_size);
    FreeResource(resource_data);

    WORD *translation;
    UINT  translation_len;
    if (!VerQueryValueW(_ver_resource, L"\\VarFileInfo\\Translation",
                        (LPVOID *)&translation, &translation_len))
    {
        LOG("cannot query Translation");
        return false;
    }

    _ver_vfi_translation = translation;
    return true;
}

static LPCWSTR
_load_string_file_info(const char *name)
{
    WCHAR path[MAX_PATH];
    swprintf(path, MAX_PATH, L"\\StringFileInfo\\%04X%04X\\%s",
             _ver_vfi_translation[0], _ver_vfi_translation[1], name);

    WCHAR *string;
    UINT   string_len;
    if (!VerQueryValueW(_ver_resource, path, (LPVOID *)&string, &string_len))
    {
        LOG("cannot query %s", name);
        return NULL;
    }

    return string;
}

const char *
pal_get_version_string(void)
{
    LOG("entry");

    if (_ver_string[0])
    {
        LOG("exit, cached");
        return _ver_string;
    }

    if (!_load_version_info())
    {
        LOG("cannot load version info");
        return "Lavender";
    }

    LPCWSTR name = _load_string_file_info("ProductName");
    if (NULL == name)
    {
        LOG("cannot load product name string");
        return "Lavender";
    }

    LPCWSTR version = _load_string_file_info("ProductVersion");
    if (NULL == version)
    {
        LOG("cannot load product version string");
        WideCharToMultiByte(CP_UTF8, 0, version, -1, _ver_string, MAX_PATH,
                            NULL, NULL);
    }
    else
    {
        snprintf(_ver_string, MAX_PATH, "%ls %ls", name, version);
    }

    LocalFree(_ver_resource);
    LOG("exit, '%s'", _ver_string);
    return _ver_string;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    return 0;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    LPWSTR wbuffer = (PWSTR)alloca(max_length * sizeof(WCHAR));
    int    length = LoadStringW(NULL, id, wbuffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    int mb_length = WideCharToMultiByte(CP_UTF8, 0, wbuffer, length, buffer,
                                        max_length, NULL, NULL);
    buffer[mb_length] = 0;

    LOG("exit, '%s'", buffer);
    return length;
}

static int CALLBACK
enum_font_fam_proc(const LOGFONTW    *logfont,
                   const TEXTMETRICW *metric,
                   DWORD              type,
                   LPARAM             lparam)
{
    if ((TRUETYPE_FONTTYPE != type) ||
        (FIXED_PITCH != (logfont->lfPitchAndFamily & 0x3)) ||
        (FW_NORMAL != logfont->lfWeight))
    {
        return 1;
    }

    *(bool *)lparam = true;
    return 1;
}

const char *
sdl2arch_get_font(void)
{
    if (_font)
    {
        return _font;
    }

    HKEY hkfonts;
    if (ERROR_SUCCESS !=
        RegOpenKeyW(HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                    &hkfonts))
    {
        LOG("cannot retrive fonts registry key");
        return NULL;
    }

    HDC   hdc = GetDC(NULL);
    bool  found = false;
    DWORD index = 0;
    WCHAR value[MAX_PATH];
    DWORD value_len = MAX_PATH;
    DWORD type;
    BYTE  data[MAX_PATH * sizeof(WCHAR)];
    DWORD data_size = sizeof(data);
    while (ERROR_SUCCESS == RegEnumValueW(hkfonts, index, value, &value_len,
                                          NULL, &type, data, &data_size))
    {
        index++;
        value_len = MAX_PATH;
        data_size = sizeof(data);

        if (REG_SZ != type)
        {
            continue;
        }

        LPWSTR ttf_suffix = wcsstr(value, L" (TrueType)");
        if (ttf_suffix)
        {
            *ttf_suffix = 0;
        }

        EnumFontFamiliesW(hdc, value, enum_font_fam_proc, (LPARAM)&found);
        if (found)
        {
            LOG("font: '%ls', file: '%ls'", value, data);
            break;
        }
    }

    RegCloseKey(hkfonts);

    if (!found)
    {
        LOG("cannot match font");
        return NULL;
    }

    WCHAR path[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT,
                                    path)))
    {
        LOG("cannot retrieve font directory path");
        return NULL;
    }

    wcsncat(path, L"\\", MAX_PATH);
    wcsncat(path, (LPWSTR)data, MAX_PATH);

    int length = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    _font = (char *)malloc(length + 1);
    if (NULL == _font)
    {
        LOG("cannot allocate buffer");
        return NULL;
    }

    WideCharToMultiByte(CP_UTF8, 0, path, -1, _font, length, NULL, NULL);
    return _font;
}

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    if (error)
    {
        swprintf(msg, MAX_PATH, L"%s\nerror %d", text, error);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);
    }

    MessageBoxW(_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}

BOOL __stdcall _imp__AttachConsole(DWORD dwProcessId)
{
    return FALSE;
}

BOOL __stdcall _imp__GetModuleHandleExW(DWORD    dwFlags,
                                        LPCWSTR  lpModuleName,
                                        HMODULE *phModule)
{
    if (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS & dwFlags)
    {
        *phModule = GetModuleHandleW(NULL);
        return TRUE;
    }

    if (GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT & dwFlags)
    {
        *phModule = GetModuleHandleW(lpModuleName);
        return TRUE;
    }

    return FALSE;
}

UINT __stdcall _imp__GetRawInputData(HRAWINPUT hRawInput,
                                     UINT      uiCommand,
                                     LPVOID    pData,
                                     PUINT     pcbSize,
                                     UINT      cbSizeHeader)
{
    return -1;
}

UINT __stdcall _imp__GetRawInputDeviceInfoA(HANDLE hDevice,
                                            UINT   uiCommand,
                                            LPVOID pData,
                                            PUINT  pcbSize)
{
    return -1;
}

UINT __stdcall _imp__GetRawInputDeviceList(
    PRAWINPUTDEVICELIST pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize)
{
    return -1;
}

BOOL __stdcall _imp__RegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices,
                                             UINT             uiNumDevices,
                                             UINT             cbSize)
{
    return FALSE;
}

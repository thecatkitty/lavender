#include <malloc.h>
#include <wchar.h>

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

extern char binary_obj_version_txt_start[];

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

const char *
pal_get_version_string(void)
{
    return binary_obj_version_txt_start;
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

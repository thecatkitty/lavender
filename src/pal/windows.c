#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define UNICODE
#include <shlobj.h>
#include <windows.h>

#include <fmt/utf8.h>
#include <pal.h>
#include <snd.h>

#if defined(CONFIG_SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <platform/sdl2arch.h>
#endif

#include "../resource.h"
#include "pal_impl.h"

#if !defined(CONFIG_SDL2)
#include "evtmouse.h"
#endif

#ifdef _MSC_VER
#define FMT_AS "%S"
#else
#define FMT_AS "%s"
#endif

static LPVOID _ver_resource = NULL;
static WORD  *_ver_vfi_translation = NULL;
static char   _ver_string[MAX_PATH] = {0};

#if defined(CONFIG_SDL2)
extern SDL_Window *_window;
#endif

static HICON         _icon = NULL;
static HWND          _wnd = NULL;
static char         *_font = NULL;
static LARGE_INTEGER _start_pc, _pc_freq;

#if !defined(CONFIG_SDL2)
static HINSTANCE _instance = NULL;
static int       _cmd_show;

static WPARAM _keycode;
#endif

extern int
__mme_init(void);

#if !defined(CONFIG_SDL2)
extern int
main(int argc, char *argv[]);

int WINAPI
wWinMain(HINSTANCE instance,
         HINSTANCE prev_instance,
         PWSTR     cmd_line,
         int       cmd_show)
{
    _instance = instance;
    _cmd_show = cmd_show;

    char  argv0[MAX_PATH];
    char *argv[] = {argv0, NULL};
    WideCharToMultiByte(CP_UTF8, 0, __wargv[0], -1, argv0, MAX_PATH, NULL,
                        NULL);
    return main(__argc, argv);
}

static LRESULT CALLBACK
_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN: {
        switch (wparam)
        {
        case VK_BACK:
        case VK_TAB:
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_END:
        case VK_HOME:
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_INSERT:
        case VK_DELETE:
        case VK_F1:
        case VK_F2:
        case VK_F3:
        case VK_F4:
        case VK_F5:
        case VK_F6:
        case VK_F7:
        case VK_F8:
        case VK_F9:
        case VK_F10:
        case VK_F11:
        case VK_F12:
        case VK_OEM_MINUS: {
            _keycode = wparam;
            return 0;
        }

        default: {
            if ((('0' <= wparam) && ('9' >= wparam)) ||
                (('A' <= wparam) && ('Z' >= wparam)))
            {
                _keycode = wparam;
            }

            return 0;
        }
        }
    }

    case WM_KEYUP: {
        _keycode = 0;
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(wnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.

        FillRect(hdc, &ps.rcPaint, GetStockObject(BLACK_BRUSH));

        EndPaint(wnd, &ps);

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wparam, lparam);
}
#endif

#if defined(_MSC_VER)
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
static void
_die(unsigned ids)
{
    WCHAR msg[MAX_PATH];
    LoadStringW(NULL, ids, msg, MAX_PATH);
    MessageBoxW(NULL, msg, L"Lavender", MB_ICONERROR);
    exit(1);
}

void
pal_initialize(int argc, char *argv[])
{
    QueryPerformanceFrequency(&_pc_freq);
    QueryPerformanceCounter(&_start_pc);

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        _die(IDS_NOARCHIVE);
    }

#if defined(CONFIG_SDL2)
    if (!sdl2arch_initialize())
    {
        LOG("SDL2 architecture initialization failed");
        _die(IDS_UNSUPPENV);
    }

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(_window, &wminfo);
    _wnd = wminfo.info.win.window;
#else
    const wchar_t wndc_name[] = L"Slideshow Window Class";
    WNDCLASS      wndc = {.lpfnWndProc = _wnd_proc,
                          .hInstance = _instance,
                          .lpszClassName = wndc_name};

    if (0 == RegisterClassW(&wndc))
    {
        LOG("cannot register the window class");
        _die(IDS_UNSUPPENV);
    }

    WCHAR title[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, pal_get_version_string(), -1, title,
                        MAX_PATH);

    _wnd = CreateWindowExW(0,         // Optional window styles
                           wndc_name, // Window class
                           title,     // Window text
                           WS_OVERLAPPEDWINDOW &
                               ~(WS_MAXIMIZEBOX | WS_SIZEBOX), // Window style
                           CW_USEDEFAULT, CW_USEDEFAULT,       // Position
                           640, 480,                           // Size
                           NULL,                               // Parent
                           NULL,                               // Menu
                           _instance, // Application instance
                           NULL);
    if (NULL == _wnd)
    {
        LOG("cannot create the window");
        UnregisterClassW(wndc_name, _instance);
        _die(IDS_UNSUPPENV);
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        DestroyWindow(_wnd);
        UnregisterClassW(wndc_name, _instance);
        _die(IDS_UNSUPPENV);
    }

    ShowWindow(_wnd, _cmd_show);
#endif

    hasset icon = pal_open_asset("windows.ico", O_RDONLY);
    if (icon)
    {
        int   small_size = GetSystemMetrics(SM_CYSMICON);
        int   large_size = GetSystemMetrics(SM_CYICON);
        char *data = pal_get_asset_data(icon);
        int   size = pal_get_asset_size(icon);

        int small_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, small_size, small_size, 0);
        int large_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, large_size, large_size, 0);
        _icon = NULL;

        SendMessageW(_wnd, WM_SETICON, ICON_BIG,
                     (LPARAM)CreateIconFromResource((PBYTE)data + large_offset,
                                                    size - large_offset, TRUE,
                                                    0x00030000));
        SendMessageW(_wnd, WM_SETICON, ICON_SMALL,
                     (LPARAM)CreateIconFromResource((PBYTE)data + small_offset,
                                                    size - small_offset, TRUE,
                                                    0x00030000));

        pal_close_asset(icon);
    }
    else
    {
        _icon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
        SendMessageW(_wnd, WM_SETICON, ICON_BIG, (LPARAM)_icon);
        SendMessageW(_wnd, WM_SETICON, ICON_SMALL, (LPARAM)_icon);
    }

    __mme_init();

    if (!snd_initialize(NULL))
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

#if defined(CONFIG_SDL2)
    sdl2arch_cleanup();
#endif
    ziparch_cleanup();
}

#if !defined(CONFIG_SDL2)
bool
pal_handle(void)
{
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (WM_QUIT == msg.message)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return true;
}

uint16_t
pal_get_keystroke(void)
{
    uint16_t keycode = _keycode;
    _keycode = 0;
    return keycode;
}
#endif

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
    swprintf(path, MAX_PATH, L"\\StringFileInfo\\%04X%04X\\" FMT_AS,
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
        WideCharToMultiByte(CP_UTF8, 0, name, -1, _ver_string, MAX_PATH, NULL,
                            NULL);
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
    DWORD volume_sn = 0;

    wchar_t self[MAX_PATH];
    if (0 == GetModuleFileNameW(NULL, self, MAX_PATH))
    {
        LOG("cannot retrieve executable path!");
        goto end;
    }

    wchar_t volume_path[MAX_PATH];
    if (!GetVolumePathNameW(self, volume_path, MAX_PATH))
    {
        LOG("cannot get volume path for '%ls'!", self);
        goto end;
    }

    wchar_t volume_name[MAX_PATH];
    if (!GetVolumeInformationW(volume_path, volume_name, MAX_PATH, &volume_sn,
                               NULL, NULL, NULL, 0))
    {
        LOG("cannot get volume information for '%ls'!", volume_path);
        goto end;
    }

    wchar_t wide_tag[12];
    if (0 == MultiByteToWideChar(CP_OEMCP, 0, tag, -1, wide_tag, 12))
    {
        LOG("cannot widen tag '%s'!", tag);
        goto end;
    }

    if (0 != wcscmp(wide_tag, volume_name))
    {
        LOG("volume name '%ls' not matching '%ls'!", volume_name, wide_tag);
        goto end;
    }

end:
    LOG("exit, %04X-%04X", HIWORD(volume_sn), LOWORD(volume_sn));
    return (uint32_t)volume_sn;
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

#if defined(CONFIG_SDL2)
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
#endif

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    if (error)
    {
        swprintf(msg, MAX_PATH, L"" FMT_AS "\nerror %d", text, error);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);
    }

    MessageBoxW(_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}

HWND
windows_get_hwnd(void)
{
    return _wnd;
}

#if !defined(CONFIG_SDL2)
void
windows_set_window_title(const char *title)
{
    WCHAR wtitle[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, MAX_PATH);
    SetWindowTextW(_wnd, wtitle);
}
#endif

#if defined(CONFIG_SDL2)
#define WINAPISHIM(dll, name, type, args, body)                                \
    type __stdcall _imp__##name args                                           \
    {                                                                          \
        static type(__stdcall *_pfn) args = NULL;                              \
        if (NULL == _pfn)                                                      \
        {                                                                      \
            HMODULE hmo = LoadLibraryW(L##dll);                                \
            if (hmo)                                                           \
            {                                                                  \
                _pfn = (type(__stdcall *) args)GetProcAddress(hmo, #name);     \
            }                                                                  \
        }                                                                      \
                                                                               \
        body;                                                                  \
    }

#define WINAPICALL(...)                                                        \
    if (_pfn)                                                                  \
    {                                                                          \
        return _pfn(__VA_ARGS__);                                              \
    }

WINAPISHIM("kernel32.dll", AttachConsole, BOOL, (DWORD dwProcessId), {
    WINAPICALL(dwProcessId);

    return FALSE;
})

WINAPISHIM("kernel32.dll",
           GetModuleHandleExW,
           BOOL,
           (DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule),
           {
               WINAPICALL(dwFlags, lpModuleName, phModule);

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
           })

WINAPISHIM("user32.dll",
           GetRawInputData,
           UINT,
           (HRAWINPUT hRawInput,
            UINT      uiCommand,
            LPVOID    pData,
            PUINT     pcbSize,
            UINT      cbSizeHeader),
           {
               WINAPICALL(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

               return -1;
           })

WINAPISHIM("user32.dll",
           GetRawInputDeviceInfoA,
           UINT,
           (HANDLE hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize),
           {
               WINAPICALL(hDevice, uiCommand, pData, pcbSize);

               return -1;
           })

WINAPISHIM("user32.dll",
           GetRawInputDeviceList,
           UINT,
           (PRAWINPUTDEVICELIST pRawInputDeviceList,
            PUINT               puiNumDevices,
            UINT                cbSize),
           {
               WINAPICALL(pRawInputDeviceList, puiNumDevices, cbSize);

               return -1;
           })

WINAPISHIM("user32.dll",
           RegisterRawInputDevices,
           BOOL,
           (PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize),
           {
               WINAPICALL(pRawInputDevices, uiNumDevices, cbSize);

               return FALSE;
           })
#endif

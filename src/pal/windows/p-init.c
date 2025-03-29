#include <stdio.h>

// clang-format off
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
// clang-format on

#include <arch/zip.h>
#include <gfx.h>
#include <pal.h>
#include <snd.h>

#include "../../resource.h"
#include "impl.h"

#define HELP_MAX_LENGTH 1024

#ifndef ICC_STANDARD_CLASSES
#define ICC_STANDARD_CLASSES 0x00004000
#endif

static HICON icon_ = NULL;

#if defined(_MSC_VER)
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
static void
die_early(unsigned ids)
{
    WCHAR msg[MAX_PATH];
    LoadStringW(NULL, ids, msg, MAX_PATH);
    MessageBoxW(NULL, msg, L"Lavender", MB_ICONERROR);
    exit(1);
}

#if defined(CONFIG_SOUND)
static bool
snddev_enum_callback(device *dev, void *data)
{
    wchar_t line[MAX_PATH];
    swprintf(line,
#if !defined(_MSC_VER) || (_MSC_VER > 1400)
             MAX_PATH,
#endif
             FMT_AS L"\t" FMT_AS L"\n", dev->name, dev->description);
    windows_append((wchar_t *)data, line, HELP_MAX_LENGTH);
    return true;
}
#endif // CONFIG_SOUND

static void
_show_help(const char *self)
{
    const char *name = NULL;

    wchar_t message[HELP_MAX_LENGTH] = L"";
    wchar_t part[MAX_PATH];

    if (NULL == (name = strrchr(self, '\\')))
    {
        name = self;
    }
    else
    {
        name++;
    }

    if (name)
    {
        MultiByteToWideChar(CP_UTF8, 0, name, -1, part, lengthof(part));
        windows_append(message, part, lengthof(message));
    }
    else
    {
        windows_append(message, L"LAVENDER", lengthof(message));
    }
    windows_append(message,
                   L" [/? | /K"
#if defined(CONFIG_SOUND)
                   L" | /S<dev>"
#endif // CONFIG_SOUND
                   L"]\n",
                   lengthof(message));

#if defined(CONFIG_SOUND)
    snd_load_inbox_drivers();

    windows_append(message, L"\ndev:\n", lengthof(message));
    snd_enum_devices(snddev_enum_callback, message);
#endif // CONFIG_SOUND

    windows_append(message, L"\n", lengthof(message));
    windows_about(L"Lavender", message);
}

void
pal_initialize(int argc, char *argv[])
{
    const wchar_t wndc_name[] = L"Slideshow Window Class";
    WCHAR         title[MAX_PATH];
    WNDCLASS      wndc = {0};

    bool   arg_kiosk;
    int    i;
    hasset icon;

#if defined(CONFIG_SOUND)
    const char *arg_snd = NULL;
#endif // CONFIG_SOUND

    windows_start_time = timeGetTime();

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        die_early(IDS_NOARCHIVE);
    }

    arg_kiosk = false;
    for (i = 1; i < argc; i++)
    {
        if ('/' != argv[i][0])
        {
            continue;
        }

        if ('k' == tolower(argv[i][1]))
        {
            arg_kiosk = true;
        }

#if defined(CONFIG_SOUND)
        if ('s' == tolower(argv[i][1]))
        {
            arg_snd = argv[i] + 2;
        }
#endif // CONFIG_SOUND

        if ('?' == argv[i][1])
        {
            _show_help(argv[0]);
            exit(1);
        }
    }

    {
        INITCOMMONCONTROLSEX icc = {sizeof(INITCOMMONCONTROLSEX),
                                    ICC_STANDARD_CLASSES};
        InitCommonControlsEx(&icc);
    }

    wndc.lpfnWndProc = windows_wndproc;
    wndc.hInstance = windows_instance;
    wndc.lpszClassName = wndc_name;
    if (0 == RegisterClassW(&wndc))
    {
        LOG("cannot register the window class");
        die_early(IDS_UNSUPPENV);
    }

    MultiByteToWideChar(CP_UTF8, 0, pal_get_version_string(), -1, title,
                        MAX_PATH);

    windows_wnd = CreateWindowExW(
        0,         // Optional window styles
        wndc_name, // Window class
        title,     // Window text
        WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX), // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,                         // Position
        640, 480,                                             // Size
        NULL,                                                 // Parent
        NULL,                                                 // Menu
        windows_instance, // Application instance
        NULL);
    if (NULL == windows_wnd)
    {
        LOG("cannot create the window");
        UnregisterClassW(wndc_name, windows_instance);
        die_early(IDS_UNSUPPENV);
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        DestroyWindow(windows_wnd);
        UnregisterClassW(wndc_name, windows_instance);
        die_early(IDS_UNSUPPENV);
    }

    windows_fullscreen = false;
    gfx_get_glyph_dimensions(&windows_cell);

    ShowWindow(windows_wnd, windows_cmd_show);
    pal_stall(-1);

    if (arg_kiosk)
    {
        windows_toggle_fullscreen(windows_wnd);
    }

    icon = pal_open_asset("windows.ico", O_RDONLY);
    if (icon)
    {
        int   small_size = GetSystemMetrics(SM_CYSMICON);
        int   large_size = GetSystemMetrics(SM_CYICON);
        char *data = pal_load_asset(icon);
        int   size = pal_get_asset_size(icon);

        int small_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, small_size, small_size, 0);
        int large_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, large_size, large_size, 0);
        icon_ = NULL;

        SendMessageW(windows_wnd, WM_SETICON, ICON_BIG,
                     (LPARAM)CreateIconFromResource((PBYTE)data + large_offset,
                                                    size - large_offset, TRUE,
                                                    0x00030000));
        SendMessageW(windows_wnd, WM_SETICON, ICON_SMALL,
                     (LPARAM)CreateIconFromResource((PBYTE)data + small_offset,
                                                    size - small_offset, TRUE,
                                                    0x00030000));

        pal_close_asset(icon);
    }
    else
    {
        icon_ = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
        SendMessageW(windows_wnd, WM_SETICON, ICON_BIG, (LPARAM)icon_);
        SendMessageW(windows_wnd, WM_SETICON, ICON_SMALL, (LPARAM)icon_);
    }

#if defined(CONFIG_SOUND)
    if (!snd_initialize(arg_snd))
    {
        LOG("cannot initialize sound");
    }
#endif // CONFIG_SOUND
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (windows_wnd)
    {
        KillTimer(windows_wnd, 0);
    }

    snd_cleanup();
    gfx_cleanup();
    ziparch_cleanup();
}

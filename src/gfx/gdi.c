#include <windows.h>

#include <gfx.h>
#include <platform/windows.h>

static HFONT _font = NULL;
static HWND  _wnd = NULL;
static SIZE  _glyph;

bool
gfx_initialize(void)
{
    _font = CreateFontW(16, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                        ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, NULL);
    _wnd = windows_get_hwnd();

    HDC dc = GetDC(_wnd);
    SelectObject(dc, _font);

    TEXTMETRICW metric;
    GetTextMetricsW(dc, &metric);
    _glyph.cx = metric.tmAveCharWidth;
    _glyph.cy = metric.tmHeight;

    ReleaseDC(_wnd, dc);

    RECT rect;
    GetClientRect(_wnd, &rect);
    rect.right = rect.left + 80 * _glyph.cx;
    rect.bottom = rect.top + 25 * _glyph.cy;

    AdjustWindowRect(&rect, GetWindowLong(_wnd, GWL_STYLE), FALSE);
    SetWindowPos(_wnd, 0, -1, -1, rect.right - rect.left,
                 rect.bottom - rect.top, SWP_NOMOVE | SWP_NOREDRAW);
    return true;
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    dim->width = _glyph.cx * 80;
    dim->height = _glyph.cy * 25;
}

void
gfx_get_glyph_dimensions(gfx_dimensions *dim)
{
    dim->width = _glyph.cx;
    dim->height = _glyph.cy;
}

uint16_t
gfx_get_pixel_aspect(void)
{
    return 64 * 1;
}

unsigned
gfx_get_color_depth(void)
{
    return 24;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    return true;
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    int    length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wstr = (LPWSTR)alloca((length + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, length + 1);

    RECT rect;
    GetClientRect(_wnd, &rect);
    rect.left += x * _glyph.cx;
    rect.top += y * _glyph.cy;

    HDC dc = GetDC(_wnd);
    SelectObject(dc, _font);

    DrawTextW(dc, wstr, -1, &rect, DT_CALCRECT | DT_SINGLELINE);
    RECT txt_rect = {0, 0, rect.right - rect.left, rect.bottom - rect.top};

    HDC     txt_dc = CreateCompatibleDC(dc);
    HBITMAP txt_bm =
        CreateCompatibleBitmap(dc, txt_rect.right, txt_rect.bottom);
    SelectObject(txt_dc, txt_bm);
    SelectObject(txt_dc, _font);
    SetBkMode(txt_dc, OPAQUE);
    SetBkColor(txt_dc, 0x000000);
    SetTextColor(txt_dc, 0xFFFFFF);
    DrawTextW(txt_dc, wstr, -1, &txt_rect, DT_SINGLELINE);

    BitBlt(dc, rect.left, rect.top, txt_rect.right, txt_rect.bottom, txt_dc, 0,
           0, SRCINVERT);
    DeleteObject(txt_bm);
    DeleteDC(txt_dc);

    ReleaseDC(_wnd, dc);
    return true;
}

void
gfx_cleanup(void)
{
    DeleteObject(_font);
}

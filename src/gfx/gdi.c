#include <windows.h>

#include <gfx.h>
#include <platform/windows.h>

static HFONT _font = NULL;
static HWND  _wnd = NULL;
static SIZE  _glyph;

static const COLORREF COLORS[] = {[GFX_COLOR_BLACK] = RGB(0, 0, 0),
                                  [GFX_COLOR_NAVY] = RGB(0, 0, 128),
                                  [GFX_COLOR_GREEN] = RGB(0, 128, 0),
                                  [GFX_COLOR_TEAL] = RGB(0, 128, 128),
                                  [GFX_COLOR_MAROON] = RGB(128, 0, 0),
                                  [GFX_COLOR_PURPLE] = RGB(128, 0, 128),
                                  [GFX_COLOR_OLIVE] = RGB(128, 128, 0),
                                  [GFX_COLOR_SILVER] = RGB(192, 192, 192),
                                  [GFX_COLOR_GRAY] = RGB(128, 128, 128),
                                  [GFX_COLOR_BLUE] = RGB(0, 0, 255),
                                  [GFX_COLOR_LIME] = RGB(0, 255, 0),
                                  [GFX_COLOR_CYAN] = RGB(0, 255, 255),
                                  [GFX_COLOR_RED] = RGB(255, 0, 0),
                                  [GFX_COLOR_FUCHSIA] = RGB(255, 0, 255),
                                  [GFX_COLOR_YELLOW] = RGB(255, 255, 0),
                                  [GFX_COLOR_WHITE] = RGB(255, 255, 255)};

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

static void
_to_wrect(const gfx_rect *rect, RECT *wrect)
{
    RECT crect;
    GetClientRect(_wnd, &crect);

    wrect->left = crect.left + rect->left;
    wrect->top = crect.top + rect->top;
    wrect->right = wrect->left + rect->width;
    wrect->bottom = wrect->top + rect->height;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    return true;
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    HDC dc = GetDC(_wnd);
    SetDCPenColor(dc, COLORS[color]);
    SelectObject(dc, GetStockObject(DC_PEN));
    MoveToEx(dc, rect->left, rect->top, NULL);
    LineTo(dc, rect->left + rect->width - 1, rect->top + rect->height - 1);
    ReleaseDC(_wnd, dc);
    return true;
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    RECT wrect;
    _to_wrect(rect, &wrect);
    wrect.left--;
    wrect.top--;
    wrect.right++;
    wrect.bottom++;

    HDC dc = GetDC(_wnd);
    SetDCBrushColor(dc, COLORS[color]);
    FrameRect(dc, &wrect, GetStockObject(DC_BRUSH));
    ReleaseDC(_wnd, dc);
    return true;
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    RECT wrect;
    _to_wrect(rect, &wrect);

    HDC dc = GetDC(_wnd);
    SetDCBrushColor(dc, COLORS[color]);
    FillRect(dc, &wrect, GetStockObject(DC_BRUSH));
    ReleaseDC(_wnd, dc);
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

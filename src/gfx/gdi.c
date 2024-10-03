#include <math.h>
#include <windows.h>

#include <gfx.h>
#include <platform/windows.h>

static HFONT _font = NULL;
static HWND  _wnd = NULL;
static SIZE  _glyph;
static SIZE  _screen;
static POINT _origin;

static float _min_scale;
static float _scale;

static HDC      _dc = NULL;
static HBITMAP  _fb = NULL;
static COLORREF _bg = RGB(0, 0, 0);

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
windows_set_font(HFONT font)
{
    if (NULL != _font)
    {
        DeleteObject(_font);
    }
    _font = font;

    HDC wnd_dc = GetDC(_wnd);
    HDC new_dc = CreateCompatibleDC(wnd_dc);
    if (NULL == new_dc)
    {
        ReleaseDC(_wnd, wnd_dc);
        return false;
    }

    SelectObject(new_dc, _font);

    SIZE old_screen;
    memcpy(&old_screen, &_screen, sizeof(SIZE));

    TEXTMETRICW metric;
    GetTextMetricsW(new_dc, &metric);
    _glyph.cx = metric.tmAveCharWidth;
    _glyph.cy = metric.tmHeight;
    _screen.cx = 80 * _glyph.cx;
    _screen.cy = 25 * _glyph.cy;
    _origin.x = 0;
    _origin.y = 0;
    _scale = (float)metric.tmHeight / 16.f;

    RECT rect;
    GetClientRect(_wnd, &rect);
    rect.right = rect.left + _screen.cx;
    rect.bottom = rect.top + _screen.cy;

    AdjustWindowRect(&rect, GetWindowLong(_wnd, GWL_STYLE), FALSE);
    SetWindowPos(_wnd, 0, -1, -1, rect.right - rect.left,
                 rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    HBITMAP new_fb = CreateCompatibleBitmap(wnd_dc, _screen.cx, _screen.cy);
    SelectObject(new_dc, new_fb);
    ReleaseDC(_wnd, wnd_dc);

    RECT screen_rect = {
        .left = 0, .top = 0, .right = _screen.cx, .bottom = _screen.cy};
    if (NULL == _dc)
    {
        FillRect(new_dc, &screen_rect, GetStockObject(BLACK_BRUSH));
    }
    else
    {
        StretchBlt(new_dc, 0, 0, _screen.cx, _screen.cy, _dc, 0, 0,
                   old_screen.cx, old_screen.cy, SRCCOPY);
        DeleteObject(_fb);
        DeleteDC(_dc);
    }
    _fb = new_fb;
    _dc = new_dc;

    RedrawWindow(_wnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

    return true;
}

bool
windows_set_scale(float scale)
{
    return windows_set_font(windows_find_font(-1, roundf(scale * 16.f)));
}

bool
gfx_initialize(void)
{
    _wnd = windows_get_hwnd();

    HDC wnd_dc = GetDC(_wnd);
    _min_scale = (float)GetDeviceCaps(wnd_dc, LOGPIXELSX) / 96.f;
    ReleaseDC(_wnd, wnd_dc);

    return windows_set_scale(_min_scale);
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    dim->width = _screen.cx;
    dim->height = _screen.cy;
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

float
gfx_get_scale(void)
{
    return _scale;
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

static void
_grow_wrect(RECT *wrect, int length)
{
    wrect->left -= length;
    wrect->top -= length;
    wrect->right += length;
    wrect->bottom += length;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    BITMAPINFO *bmi = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFOHEADER) +
                                                  16 * sizeof(COLORREF));
    if (NULL == bmi)
    {
        return false;
    }

    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = bm->width;
    bmi->bmiHeader.biHeight = -bm->height;
    bmi->bmiHeader.biPlanes = bm->planes;
    bmi->bmiHeader.biBitCount = bm->bpp;
    bmi->bmiHeader.biCompression = BI_RGB;

    HDC     bmp_dc = NULL;
    HBITMAP bmp = NULL;

    // Prepare pixel data
    const void *bits = bm->bits;
    if (1 == bm->bpp)
    {
        // Prepare a DWORD-aligned buffer for 1bpp bitmaps
        if (bm->opl % 4)
        {
            size_t aligned_opl = align(bm->opl, 4);
            size_t lines = abs(bm->height);

            char *aligned_bits = malloc(aligned_opl * lines);
            if (NULL == aligned_bits)
            {
                goto end;
            }

            const char *src = bm->bits;
            char       *dst = aligned_bits;
            for (size_t i = 0; i < lines; i++)
            {
                memcpy(dst, src, bm->opl);
                src += bm->opl;
                dst += aligned_opl;
            }

            bits = aligned_bits;
        }

        // Initialize the palette for 1bpp bitmaps
        bmi->bmiColors[1].rgbRed = 255;
        bmi->bmiColors[1].rgbGreen = 255;
        bmi->bmiColors[1].rgbBlue = 255;
    }
    else if (4 == bm->bpp)
    {
        // Initialize the palette for 4bpp bitmaps
        memcpy(bmi->bmiColors, COLORS, 16 * sizeof(COLORREF));
    }

    bmp = CreateDIBitmap(_dc, &bmi->bmiHeader, CBM_INIT, bits, bmi,
                         DIB_RGB_COLORS);
    if (NULL == bmp)
    {
        goto end;
    }

    bmp_dc = CreateCompatibleDC(_dc);
    if (NULL == bmp_dc)
    {
        goto end;
    }

    int width = _scale * bm->width;
    int height = _scale * abs(bm->height);
    SelectObject(bmp_dc, bmp);
    StretchBlt(_dc, x, y, width, height, bmp_dc, 0, 0, bm->width,
               abs(bm->height), SRCCOPY);

    RECT rect = {x, y, x + width, y + height};
    OffsetRect(&rect, _origin.x, _origin.y);
    InvalidateRect(_wnd, &rect, FALSE);

end:
    if (NULL != bmp_dc)
    {
        DeleteDC(bmp_dc);
    }

    if (NULL != bmp)
    {
        DeleteObject(bmp);
    }

    if ((NULL != bits) && (bm->bits != bits))
    {
        free((void *)bits);
    }

    free(bmi);
    return true;
}

static HPEN
_get_pen(gfx_color color)
{
    LOGBRUSH logbrush = {BS_SOLID, COLORS[color]};
    return ExtCreatePen(PS_GEOMETRIC | PS_ENDCAP_SQUARE | PS_JOIN_MITER, _scale,
                        &logbrush, 0, NULL);
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    HPEN pen = _get_pen(color);
    HPEN old_pen = (HPEN)SelectObject(_dc, _get_pen(color));
    MoveToEx(_dc, rect->left, rect->top, NULL);
    LineTo(_dc, rect->left + rect->width - 1, rect->top + rect->height - 1);
    SelectObject(_dc, old_pen);
    DeleteObject(pen);

    RECT wrect;
    _to_wrect(rect, &wrect);
    _grow_wrect(&wrect, _scale);
    OffsetRect(&wrect, _origin.x, _origin.y);
    InvalidateRect(_wnd, &wrect, FALSE);
    return true;
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    RECT wrect;
    _to_wrect(rect, &wrect);
    _grow_wrect(&wrect, 1);

    HPEN   pen = _get_pen(color);
    HPEN   old_pen = (HPEN)SelectObject(_dc, _get_pen(color));
    HBRUSH old_brush = (HBRUSH)SelectObject(_dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(_dc, wrect.left, wrect.top, wrect.right, wrect.bottom);
    SelectObject(_dc, old_brush);
    SelectObject(_dc, old_pen);
    DeleteObject(pen);

    _grow_wrect(&wrect, _scale);
    OffsetRect(&wrect, _origin.x, _origin.y);
    InvalidateRect(_wnd, &wrect, FALSE);
    return true;
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    RECT wrect;
    _to_wrect(rect, &wrect);

    SetDCBrushColor(_dc, COLORS[color]);
    FillRect(_dc, &wrect, GetStockObject(DC_BRUSH));

    if ((_screen.cx == rect->width) && (_screen.cy == rect->height))
    {
        HDC  wnd_dc = GetDC(_wnd);
        SetDCBrushColor(wnd_dc, COLORS[color]);

        RECT wnd_rect;
        GetClientRect(_wnd, &wnd_rect);
        FillRect(wnd_dc, &wnd_rect, GetStockObject(DC_BRUSH));

        ReleaseDC(_wnd, wnd_dc);
        _bg = COLORS[color];
    }
    else
    {
        OffsetRect(&wrect, _origin.x, _origin.y);
        InvalidateRect(_wnd, &wrect, FALSE);
    }
    return true;
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    size_t length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wstr = (LPWSTR)alloca((length + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, length + 1);

    RECT rect;
    GetClientRect(_wnd, &rect);
    rect.left += x * _glyph.cx;
    rect.top += y * _glyph.cy;

    SelectObject(_dc, _font);
    DrawTextW(_dc, wstr, -1, &rect, DT_CALCRECT | DT_SINGLELINE);
    RECT txt_rect = {0, 0, rect.right - rect.left, rect.bottom - rect.top};

    HDC     txt_dc = CreateCompatibleDC(_dc);
    HBITMAP txt_bm =
        CreateCompatibleBitmap(_dc, txt_rect.right, txt_rect.bottom);
    SelectObject(txt_dc, txt_bm);
    SelectObject(txt_dc, _font);
    SetBkMode(txt_dc, OPAQUE);
    SetBkColor(txt_dc, 0x000000);
    SetTextColor(txt_dc, 0xFFFFFF);
    DrawTextW(txt_dc, wstr, -1, &txt_rect, DT_SINGLELINE);

    BitBlt(_dc, rect.left, rect.top, txt_rect.right, txt_rect.bottom, txt_dc, 0,
           0, SRCINVERT);
    DeleteObject(txt_bm);
    DeleteDC(txt_dc);

    OffsetRect(&rect, _origin.x, _origin.y);
    InvalidateRect(_wnd, &rect, FALSE);
    return true;
}

void
gfx_cleanup(void)
{
    DeleteObject(_fb);
    DeleteDC(_dc);
    DeleteObject(_font);
}

bool
gfx_set_title(const char *title)
{
    WCHAR wtitle[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, MAX_PATH);
    SetWindowTextW(_wnd, wtitle);
    return true;
}

HDC
windows_get_dc(void)
{
    return _dc;
}

void
windows_set_box(int width, int height)
{
    _origin.x = (width - _screen.cx) / 2;
    _origin.y = (height - _screen.cy) / 2;
}

void
windows_get_origin(POINT *origin)
{
    memcpy(origin, &_origin, sizeof(_origin));
}

COLORREF
windows_get_bg(void)
{
    return _bg;
}

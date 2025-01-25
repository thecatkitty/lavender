#include <arch/windows.h>

static const wchar_t *FONT_NAMES[] = {
    L"Cascadia Code",  // Windows 11
    L"Consolas",       // Windows Vista
    L"Lucida Console", // Windows 2000
    NULL               // usually Courier New
};

HFONT
windows_find_font(int max_width, int max_height)
{

    HDC            dc;
    HFONT          font = NULL;
    TEXTMETRICW    metric;
    const wchar_t *family = NULL;
    int            i, min_height;

    // Determine the font family
    for (i = 0; i < lengthof(FONT_NAMES); i++)
    {
        font = CreateFontW(max_height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN,
                           FONT_NAMES[i]);
        if (NULL != font)
        {
            family = FONT_NAMES[i];
            break;
        }
    }

    if (NULL == font)
    {
        return NULL;
    }

    dc = windows_get_dc();

    // Try the largest height
    SelectObject(dc, font);
    GetTextMetricsW(dc, &metric);
    if ((0 > max_width) || (max_width >= metric.tmAveCharWidth))
    {
        return font;
    }

    // Continue with binary search, fit width
    min_height = 0;
    while (1 < (max_height - min_height))
    {
        int height = (min_height + max_height) / 2;
        DeleteObject(font);

        font =
            CreateFontW(height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                        ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, family);
        SelectObject(dc, font);
        GetTextMetricsW(dc, &metric);
        if (max_width < metric.tmAveCharWidth)
        {
            max_height = height;
        }
        else
        {
            min_height = height;
        }
    }

    DeleteObject(font);
    font = CreateFontW(min_height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, family);
    return font;
}

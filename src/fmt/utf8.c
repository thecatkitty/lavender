#include <fmt/utf8.h>

uint16_t
utf8_get_codepoint(const char *sequence, int *length)
{
    unsigned lead = *sequence & 0xC0;
    int      continuation, i;
    uint16_t cp;

    if (0x80 == lead)
    {
        errno = EILSEQ;
        *length = 0;
        return 0;
    }

    if (0xC0 != lead)
    {
        *length = 1;
        return *sequence;
    }

    // At first, assume two-byte sequence
    continuation = 1;
    cp = *sequence & 0x1F;

    if (0 != (*sequence & 0x20))
    {
        // Then assume three-byte sequence
        continuation++;
        cp &= 0x0F;

        if (0 != (*sequence & 0x10))
        {
            // Then assume four-byte sequence
            continuation++;
            cp &= 0x07;

            if (0 != (*sequence & 0x08))
            {
                errno = EILSEQ;
                *length = 0;
                return 0;
            }
        }
    }

    for (i = 1; i <= continuation; i++)
    {
        if (0x80 != (sequence[i] & 0xC0))
        {
            errno = EILSEQ;
            *length = 0;
            return 0;
        }

        cp = (cp << 6) | (sequence[i] & 0x3F);
    }

    *length = continuation + 1;
    return cp;
}

int
utf8_get_sequence(uint16_t wc, char *buff)
{
    if (0x0080 > wc)
    {
        buff[0] = (char)wc;
        return 1;
    }

    if (0x0800 > wc)
    {
        buff[0] = 0xC0 | (wc >> 6);
        buff[1] = 0x80 | (wc & 0x3F);
        return 2;
    }

    buff[0] = 0xE0 | (wc >> 12);
    buff[1] = 0x80 | ((wc >> 6) & 0x3F);
    buff[2] = 0x80 | (wc & 0x3F);
    return 3;
}

int
utf8_encode(const char *src, char *dst, char (*encoder)(uint16_t))
{
    int i;
    for (i = 0; *src; i++)
    {
        int      length;
        uint16_t code = utf8_get_codepoint(src, &length);
        if (0 == length)
        {
            return -1;
        }

        dst[i] = encoder(code);
        src += length;
    }

    dst[i] = 0;
    return i;
}

static const uint16_t _FOLDING_SINGLE[] = {
    0x00B5, 0x03BC, // MICRO SIGN
    0x0178, 0x00FF, // LATIN CAPITAL LETTER Y WITH DIAERESIS
    0x0179, 0x017A, // LATIN CAPITAL LETTER Z WITH ACUTE
    0x017B, 0x017C, // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    0x017D, 0x017E, // LATIN CAPITAL LETTER Z WITH CARON
    0x017F, 0x0073, // LATIN SMALL LETTER LONG S
    0x0181, 0x0253, // LATIN CAPITAL LETTER B WITH HOOK
    0x0182, 0x0183, // LATIN CAPITAL LETTER B WITH TOPBAR
    0x0184, 0x0185, // LATIN CAPITAL LETTER TONE SIX
    0x0186, 0x0254, // LATIN CAPITAL LETTER OPEN O
    0x0187, 0x0188, // LATIN CAPITAL LETTER C WITH HOOK
    0x0189, 0x0256, // LATIN CAPITAL LETTER AFRICAN D
    0x018A, 0x0257, // LATIN CAPITAL LETTER D WITH HOOK
    0x018B, 0x018C, // LATIN CAPITAL LETTER D WITH TOPBAR
    0x018E, 0x01DD, // LATIN CAPITAL LETTER REVERSED E
    0x018F, 0x0259, // LATIN CAPITAL LETTER SCHWA
    0x0190, 0x025B, // LATIN CAPITAL LETTER OPEN E
    0x0191, 0x0192, // LATIN CAPITAL LETTER F WITH HOOK
    0x0193, 0x0260, // LATIN CAPITAL LETTER G WITH HOOK
    0x0194, 0x0263, // LATIN CAPITAL LETTER GAMMA
    0x0196, 0x0269, // LATIN CAPITAL LETTER IOTA
    0x0197, 0x0268, // LATIN CAPITAL LETTER I WITH STROKE
    0x0198, 0x0199, // LATIN CAPITAL LETTER K WITH HOOK
    0x019C, 0x026F, // LATIN CAPITAL LETTER TURNED M
    0x019D, 0x0272, // LATIN CAPITAL LETTER N WITH LEFT HOOK
    0x019F, 0x0275, // LATIN CAPITAL LETTER O WITH MIDDLE TILDE
    0x01A0, 0x01A1, // LATIN CAPITAL LETTER O WITH HORN
    0x01A2, 0x01A3, // LATIN CAPITAL LETTER OI
    0x01A4, 0x01A5, // LATIN CAPITAL LETTER P WITH HOOK
    0x01A6, 0x0280, // LATIN LETTER YR
    0x01A7, 0x01A8, // LATIN CAPITAL LETTER TONE TWO
    0x01A9, 0x0283, // LATIN CAPITAL LETTER ESH
    0x01AC, 0x01AD, // LATIN CAPITAL LETTER T WITH HOOK
    0x01AE, 0x0288, // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
    0x01AF, 0x01B0, // LATIN CAPITAL LETTER U WITH HORN
    0x01B1, 0x028A, // LATIN CAPITAL LETTER UPSILON
    0x01B2, 0x028B, // LATIN CAPITAL LETTER V WITH HOOK
    0x01B3, 0x01B4, // LATIN CAPITAL LETTER Y WITH HOOK
    0x01B5, 0x01B6, // LATIN CAPITAL LETTER Z WITH STROKE
    0x01B7, 0x0292, // LATIN CAPITAL LETTER EZH
    0x01B8, 0x01B9, // LATIN CAPITAL LETTER EZH REVERSED
    0x01BC, 0x01BD, // LATIN CAPITAL LETTER TONE FIVE
    0x01C4, 0x01C6, // LATIN CAPITAL LETTER DZ WITH CARON
    0x01C5, 0x01C6, // LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
    0x01C7, 0x01C9, // LATIN CAPITAL LETTER LJ
    0x01C8, 0x01C9, // LATIN CAPITAL LETTER L WITH SMALL LETTER J
    0x01CA, 0x01CC, // LATIN CAPITAL LETTER NJ
    0x01F1, 0x01F3, // LATIN CAPITAL LETTER DZ
    0x01F2, 0x01F3, // LATIN CAPITAL LETTER D WITH SMALL LETTER Z
    0x01F4, 0x01F5, // LATIN CAPITAL LETTER G WITH ACUTE
    0x01F6, 0x0195, // LATIN CAPITAL LETTER HWAIR
    0x01F7, 0x01BF, // LATIN CAPITAL LETTER WYNN
    0x0220, 0x019E, // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
    0x023A, 0x2C65, // LATIN CAPITAL LETTER A WITH STROKE
    0x023B, 0x023C, // LATIN CAPITAL LETTER C WITH STROKE
    0x023D, 0x019A, // LATIN CAPITAL LETTER L WITH BAR
    0x023E, 0x2C66, // LATIN CAPITAL LETTER T WITH DIAGONAL STROKE
    0x0241, 0x0242, // LATIN CAPITAL LETTER GLOTTAL STOP
    0x0243, 0x0180, // LATIN CAPITAL LETTER B WITH STROKE
    0x0244, 0x0289, // LATIN CAPITAL LETTER U BAR
    0x0245, 0x028C, // LATIN CAPITAL LETTER TURNED V
    0xFFFF};

static const uint16_t _FOLDING_DOUBLE[] = {
    0x00DF, 0x0073, 0x0073, // LATIN SMALL LETTER SHARP S
    0x0130, 0x0069, 0x0307, // LATIN CAPITAL LETTER I WITH DOT ABOVE
    0x0149, 0x02BC, 0x006E, // LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
    0x01F0, 0x006A, 0x030C, // LATIN SMALL LETTER J WITH CARON
    0xFFFF};

// Get case folding for Unicode code point
// Returns length of folding in Unicode code points
static int
_fold_case(uint16_t cp, uint16_t *buff)
{
    const uint16_t *mapping;

    buff[0] = cp;
    buff[1] = 0;

    // NULL CHARACTER -- AT SIGN
    if ((cp < 0x0041)
        // LEFT SQUARE BRACKET -- INVERTED QUESTION MARK
        || ((cp >= 0x005B) && (cp <= 0x00BF)))
    {
        return 1;
    }

    // LATIN CAPITAL LETTER A -- LATIN CAPITAL LETTER Z
    if (((cp >= 0x0041) && (cp <= 0x005A)) ||
        ((cp >= 0x00C0) && (cp <= 0x00DE)))
    {
        buff[0] += 32;
        return 1;
    }

    // LATIN CAPITAL LETTER A WITH MACRON -- LATIN SMALL LETTER I WITH OGONEK
    if (((cp >= 0x0100) && (cp <= 0x012F))
        // LATIN CAPITAL LIGATURE IJ -- LATIN SMALL LETTER K WITH CEDILLA
        || ((cp >= 0x0132) && (cp <= 0x0137))
        // LATIN CAPITAL LETTER ENG -- LATIN SMALL LETTER Y WITH CIRCUMFLEX
        || ((cp >= 0x014A) && (cp <= 0x0177))
        // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON -- LATIN SMALL
        // LETTER EZH WITH CARON
        || ((cp >= 0x01DE) && (cp <= 0x01EF))
        // LATIN CAPITAL LETTER N WITH GRAVE -- LATIN SMALL LETTER H WITH CARON
        || ((cp >= 0x01F8) && (cp <= 0x021F))
        // LATIN CAPITAL LETTER OU -- LATIN SMALL LETTER Y WITH MACRON
        || ((cp >= 0x0222) && (cp <= 0x0233))
        // LATIN CAPITAL LETTER E WITH STROKE -- LATIN SMALL LETTER Y WITH
        // STROKE
        || ((cp >= 0x0246) && (cp <= 0x024F)))
    {
        buff[0] |= 1;
        return 1;
    }

    // LATIN CAPITAL LETTER L WITH ACUTE -- LATIN SMALL LETTER N WITH CARON
    if (((cp >= 0x0139) && (cp <= 0x0148))
        // LATIN CAPITAL LETTER A WITH CARON -- LATIN SMALL LETTER U WITH
        // DIAERESIS AND GRAVE
        || ((cp >= 0x01CD) && (cp <= 0x01DC)))
    {
        buff[0] += buff[0] % 2;
        return 1;
    }

    mapping = _FOLDING_SINGLE;
    while (mapping[0] != 0xFFFF)
    {
        if (cp == mapping[0])
        {
            buff[0] = mapping[1];
            return 1;
        }

        mapping += 2;
    }

    mapping = _FOLDING_DOUBLE;
    while (mapping[0] != 0xFFFF)
    {
        if (cp == mapping[0])
        {
            buff[0] = mapping[1];
            buff[1] = mapping[2];
            return 2;
        }

        mapping += 3;
    }

    return 1;
}

int
utf8_strlen(const char *str)
{
    int i;
    for (i = 0; *str; i++)
    {
        int length;
        utf8_get_codepoint(str, &length);
        if (0 == length)
        {
            return -1;
        }

        str += length;
    }

    return i;
}

int
utf8_strncasecmp(const char *str1, const char *str2, unsigned length)
{
    uint16_t code1, code2;
    int      length1, length2, index;
    uint16_t fold1[2], fold2[2];
    unsigned i;

    errno = 0;
    for (i = 0; i < length; i++)
    {
        code1 = utf8_get_codepoint(str1, &length1);
        code2 = utf8_get_codepoint(str2, &length2);

        _fold_case(code1, fold1);
        _fold_case(code2, fold2);

        index = fold1[0] == fold2[0];
        if (fold1[index] < fold2[index])
        {
            return -1;
        }

        if (fold1[index] > fold2[index])
        {
            return 1;
        }

        str1 += length1;
        str2 += length2;
    }

    return 0;
}

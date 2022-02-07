#include <stdint.h>

#include <ker.h>

static const uint16_t FOLDING_SINGLE[] = {
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

static const uint16_t FOLDING_DOUBLE[] = {
    0x00DF, 0x0073, 0x0073, // LATIN SMALL LETTER SHARP S
    0x0130, 0x0069, 0x0307, // LATIN CAPITAL LETTER I WITH DOT ABOVE
    0x0149, 0x02BC, 0x006E, // LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
    0x01F0, 0x006A, 0x030C, // LATIN SMALL LETTER J WITH CARON
    0xFFFF};

static int
FoldCase(uint16_t codePoint, uint16_t *buff);

int
KerGetCharacterFromUtf8(const char *sequence, uint16_t *codePoint)
{
    unsigned leadBits = *sequence & 0b11000000;

    if (0b10000000 == leadBits)
    {
        ERR(KER_INVALID_SEQUENCE);
    }

    if (0b11000000 != leadBits)
    {
        *codePoint = *sequence;
        return (0 == *codePoint) ? 0 : 1;
    }

    // At first, assume two-byte sequence
    int continuationBytes = 1;
    *codePoint = *sequence & 0b00011111;

    if (0 != (*sequence & 0b00100000))
    {
        // Then assume three-byte sequence
        continuationBytes++;
        *codePoint &= 0b00001111;

        if (0 != (*sequence & 0b00010000))
        {
            // Then assume four-byte sequence
            continuationBytes++;
            *codePoint &= 0b00000111;

            if (0 != (*sequence & 0b00001000))
            {
                ERR(KER_INVALID_SEQUENCE);
            }
        }
    }

    for (int i = 1; i <= continuationBytes; i++)
    {
        if (0b10000000 != (sequence[i] & 0b11000000))
        {
            ERR(KER_INVALID_SEQUENCE);
        }

        *codePoint = (*codePoint << 6) | (sequence[i] & 0b00111111);
    }

    return continuationBytes + 1;
}

int
KerConvertFromUtf8(const char *src, char *dst, char (*encoder)(uint16_t))
{
    int i;
    for (i = 0; *src; i++)
    {
        uint16_t code;
        int      length = KerGetCharacterFromUtf8(src, &code);
        if (0 > length)
        {
            return length;
        }

        dst[i] = encoder(code);
        src += length;
    }

    dst[i] = 0;
    return i;
}

int
KerCompareUtf8IgnoreCase(const char *str1, const char *str2, unsigned length)
{
    uint16_t code1, code2;
    int      length1, length2;
    uint16_t fold1[2], fold2[2];

    for (unsigned i = 0; i < length; i++)
    {
        if (0 > (length1 = KerGetCharacterFromUtf8(str1, &code1)))
        {
            return length1;
        }

        if (0 > (length2 = KerGetCharacterFromUtf8(str2, &code2)))
        {
            return length2;
        }

        if (FoldCase(code1, fold1) != FoldCase(code2, fold2))
        {
            return 1;
        }

        if ((fold1[0] != fold2[0]) || (fold1[1] != fold2[1]))
        {
            return 1;
        }

        str1 += length1;
        str2 += length2;
    }

    return 0;
}

// Get case folding for Unicode code point
// Returns length of folding in Unicode code points
int
FoldCase(uint16_t codePoint, uint16_t *buff)
{
    buff[0] = codePoint;
    buff[1] = 0;

    // NULL CHARACTER -- AT SIGN
    if ((codePoint < 0x0041)
        // LEFT SQUARE BRACKET -- INVERTED QUESTION MARK
        || ((codePoint >= 0x005B) && (codePoint <= 0x00BF)))
    {
        return 1;
    }

    // LATIN CAPITAL LETTER A -- LATIN CAPITAL LETTER Z
    if (((codePoint >= 0x0041) && (codePoint <= 0x005A)) ||
        ((codePoint >= 0x00C0) && (codePoint <= 0x00DE)))
    {
        buff[0] += 32;
        return 1;
    }

    // LATIN CAPITAL LETTER A WITH MACRON -- LATIN SMALL LETTER I WITH OGONEK
    if (((codePoint >= 0x0100) && (codePoint <= 0x012F))
        // LATIN CAPITAL LIGATURE IJ -- LATIN SMALL LETTER K WITH CEDILLA
        || ((codePoint >= 0x0132) && (codePoint <= 0x0137))
        // LATIN CAPITAL LETTER ENG -- LATIN SMALL LETTER Y WITH CIRCUMFLEX
        || ((codePoint >= 0x014A) && (codePoint <= 0x0177))
        // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON -- LATIN SMALL
        // LETTER EZH WITH CARON
        || ((codePoint >= 0x01DE) && (codePoint <= 0x01EF))
        // LATIN CAPITAL LETTER N WITH GRAVE -- LATIN SMALL LETTER H WITH CARON
        || ((codePoint >= 0x01F8) && (codePoint <= 0x021F))
        // LATIN CAPITAL LETTER OU -- LATIN SMALL LETTER Y WITH MACRON
        || ((codePoint >= 0x0222) && (codePoint <= 0x0233))
        // LATIN CAPITAL LETTER E WITH STROKE -- LATIN SMALL LETTER Y WITH
        // STROKE
        || ((codePoint >= 0x0246) && (codePoint <= 0x024F)))
    {
        buff[0] |= 1;
        return 1;
    }

    // LATIN CAPITAL LETTER L WITH ACUTE -- LATIN SMALL LETTER N WITH CARON
    if (((codePoint >= 0x0139) && (codePoint <= 0x0148))
        // LATIN CAPITAL LETTER A WITH CARON -- LATIN SMALL LETTER U WITH
        // DIAERESIS AND GRAVE
        || ((codePoint >= 0x01CD) && (codePoint <= 0x01DC)))
    {
        buff[0] += buff[0] % 2;
        return 1;
    }

    const uint16_t *mapping = FOLDING_SINGLE;
    while (mapping[0] != 0xFFFF)
    {
        if (codePoint == mapping[0])
        {
            buff[0] = mapping[1];
            return 1;
        }

        mapping += 2;
    }

    mapping = FOLDING_DOUBLE;
    while (mapping[0] != 0xFFFF)
    {
        if (codePoint == mapping[0])
        {
            buff[0] = mapping[1];
            buff[1] = mapping[2];
            return 2;
        }

        mapping += 3;
    }

    return 1;
}

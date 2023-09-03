#ifndef _FMT_UTF8_H_
#define _FMT_UTF8_H_

#if defined(__linux__) || defined(__MINGW32__)
#define UTF8_NATIVE
#endif

#include <base.h>

// Get code point from UTF-8 sequence
// Returns the code point (0 on NUL or error)
// *length will contain the length of the sequence (0 on error)
extern uint16_t
utf8_get_codepoint(const char *sequence, int *length);

// Get UTF-8 sequence from code point
// Returns the length of the sequence
extern int
utf8_get_sequence(uint16_t wc, char *buff);

// Convert string from UTF-8 to another encoding
// Returns the length of the converted string, negative on error
extern int
utf8_encode(const char *src, char *dst, char (*encoder)(uint16_t));

// Get character count of UTF-8 string
// Returns the length of the string, negative on error
extern int
utf8_strlen(const char *str);

// Conduct a case-insensitive comparison of two UTF-8 strings
// Returns 0 when equal, non-zero on difference
// Clears errno on success
extern int
utf8_strncasecmp(const char *str1, const char *str2, unsigned length);
#endif // _FMT_UTF8_H_

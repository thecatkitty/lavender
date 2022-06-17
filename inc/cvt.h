#ifndef _CVT_H_
#define _CVT_H_

#include <base.h>

// Get code point from UTF-8 sequence
// Returns the code point (0 on NUL or error)
// *length will contain the length of the sequence (0 on error)
extern uint16_t
cvt_utf8_get_codepoint(const char *sequence, int *length);

// Convert string from UTF-8 to another encoding
// Returns the length of the converted string, negative on error
extern int
cvt_utf8_encode(const char *src, char *dst, char (*encoder)(uint16_t));

// Conduct a case-insensitive comparison of two UTF-8 strings
// Returns 0 when equal, non-zero on difference
// Clears errno on success
extern int
cvt_utf8_strncasecmp(const char *str1, const char *str2, unsigned length);

#endif // _CVT_H_

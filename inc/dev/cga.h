#ifndef _DEV_CGA_H_
#define _DEV_CGA_H_

// Video memory base
#define CGA_HIMONO_MEM 0xB800

// Odd plane offset
#define CGA_HIMONO_PLANE 0x2000

// Text mode character height
#define CGA_CHARACTER_HEIGHT 8

// IVT pointer to font data over code point 127
#define INT_CGA_EXTENDED_FONT_PTR 31

// 0FFA6Eh - ROM font base
#define CGA_BASIC_FONT_SEGMENT 0xFFA0
#define CGA_BASIC_FONT_OFFSET  0x6E

#endif // _DEV_CGA_H_

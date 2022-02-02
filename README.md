# Lavender [![Build Status](https://dev.azure.com/celones/thecatkitty-gh-public/_apis/build/status/thecatkitty.lavender?branchName=main)](https://dev.azure.com/celones/thecatkitty-gh-public/_build/latest?definitionId=3&branchName=main)

A simple configurable slideshow program for DOS 2.0+ written in C and some 8086 assembly. It shows slides consisting of text and graphics contained in the ZIP file appended to the executable.

This is still work in progress, but I'm doing my best to separate working version (`main` branch) from progressing code (PRs).

## Current features
* CGA 640x200 monochrome video mode
* display delays
* displaying text (with UTF-8 subset support)
* displaying PBM bitmaps
* drawing and filling rectangles
* jumping between slides with key presses
* inclusion of other scripts from within a script
* PC speaker music playback
* reporting the unsupported environment (DOS 1.x, Windows Vista and newer)

## Building
Building requires Linux with *GNU Make*, *GNU Binutils*, [GCC for IA-16](https://github.com/tkchia/gcc-ia16/) with [libi86](https://github.com/tkchia/libi86/), and `zip`. If you have it, just run:
```sh
make
```

Building additional tools requires GCC and MinGW-w64.

If you want to change the output slideshow application file name (`sshow.com` is default) or the slideshow source directory, you can set `LAV_SSHOW` and `LAV_DATA` environmental variables respectively.

## Format of `slides.txt`
The slideshow file consists of entries specifying various actions. Each entry ends with CRLF. Blank lines can be used for clarity.

- `<delay>` is a decimal integer of milliseconds that need to pass before action
- `<label>` is an arbitrary text up to 255 bytes, so is `<content>`
- `<alignemnt>` is either `<` for left, `^` for center, or `>` for right
- `<top>` and `<left>` set vertical and horizontal position in pixels
- `<width>` and `<height>` set width and height in pixels

### Label
```
:<label>
```

### Text
```
<delay> T <line> <column> <content>
- or -
<delay> T <line> <alignment><content>
```

`<line>` sets the vertical position in lines.
`<column>` sets the horizontal position in columns.

### Bitmap
```
<delay> B <top> <left> <name pattern>
- or -
<delay> B <top> <alignment><name pattern>
```

`<name pattern>` sets the bitmap file name, or file name pattern in case of pixel aspect ratio dependent images (in such case `<>` is a placeholder for encoding PAR).

### Rectangle
```
<delay> R|r <top> <left|alignment> <width> <height> <color>
```

`R` is rectangle border, `r` is rectangle fill. In border width, height, and position refer to the inner box.
`<color>` is either `B` for black or `W` for white.

### Play music
```
<delay> P <file name>
```
`<file name>` refers to the music file name.

### Wait for key press
```
<delay> K
```

Stores the scan code in the *Accumulator*.

#### Unconditional jump
```
<delay> J <label>
```

### Conditional jump
```
<delay> = <value> <label>
```

Jumps to `<label>` if the *Accumulator* is equal to the `<value>`.

### Script call
```
<delay> ! <script>
- or -
<delay> ! <script> <method> <crc32> <parameter> <data>
```

Calls a script file named `<script>` as plain text or using some encryption method.
`<method>` is 0 for plain text, 1 for XOR48.
`<crc32>` is hexadecimal.
`<parameter>` is a decimal integer.
`<data>` is up to 63 characters.

## Format of the PC speaker music file
```
Header:
    0x03 0x3C
Note:
    <duration: BYTE> <divisor: WORD>
Silence:
    <duration: BYTE> 0x00 0x00
End:
    0x00
```

`<duration>` is measured in sixtieths of a second.
`<divisor>` sets the pitch and relates to the Programmable Interval Timer input frequency (1193181.6667 Hz).

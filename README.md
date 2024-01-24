# Lavender ![Build status badge](https://github.com/thecatkitty/lavender/actions/workflows/build.yml/badge.svg?event=push)

A simple configurable slideshow program for DOS 2.0+ and Windows 2000+ written mainly in C. It shows slides consisting of text and graphics contained in the ZIP file appended to the executable.

This is still work in progress, but I'm doing my best to separate working version (`main` branch) from progressing code (PRs).

![Lavender project logo](docs/lavender.png)

## Current features
* DOS, Windows, and diagnostic Linux targets
* CGA 640x200 monochrome video mode under DOS
* display delays (animations)
* displaying text (with UTF-8 subset support)
* displaying PBM bitmaps
* drawing and filling rectangles
* jumping between slides with key presses and mouse clicks
* inclusion of other scripts from within a script
* PC speaker music playback (including limited support for Standard MIDI Files)
* reporting the unsupported environment (DOS 1.x, Windows Vista and newer)
* multiple language support (Czech, English, Polish)

## Building
Building requires x86_64 Linux with *CMake*, *GNU Make*, *GNU Binutils*, *MinGW-w64* for i686 and x86_64 with SDL2, SDL2_ttf, Fontconfig, and FluidSynth, [GCC for IA-16](https://github.com/tkchia/gcc-ia16/) with [libi86](https://github.com/tkchia/libi86/), and `zip`. If you have it all, you can configure the environment:
```sh
# Native (diagnostic) Linux build, either GCC or Clang is fine
cmake -S . -B build

# MS-DOS build, COM file (DOS 2+)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/DOS-GCC-IA16.cmake -DDOS_TARGET_COM=1

# MS-DOS build, EXE file (DOS 3+)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/DOS-GCC-IA16.cmake

# Windows build, IA-32 (Windows 2000+)
CC=i686-w64-mingw32-gcc cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows

# Windows build, x64 (Windows XP+)
CC=x86_64-w64-mingw32-gcc cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows
```

You can select user interface language (`ENU`, `CSY`, `PLK`) by setting adding the `-DLAV_LANG=` option.

After finishing configuration, navigate to the `build` directory and run `make bundle`. An executable called `sshow` should appear. You can modify the slideshow by editing files in the `data` directory.

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
`<color>` is `B` for black, `W` for white, or `G` for _gray_ (checkered pattern).

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

### Define an active (clickable) area
```
<delay> A <line> <column> <width> <height> <tag>
```

When the area is clicked, `<tag>` is stored in the *Accumulator*.
In order to remove an area, set its `<tag>` to zero.

In order to remove all active areas:
```
<delay> A 0 0 0 0 0
```

### Unconditional jump
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

# Lavender ![Build status badge](https://github.com/thecatkitty/lavender/actions/workflows/build.yml/badge.svg?event=push)

A simple configurable slideshow program for DOS 2.0+ and Windows 2000+ written mainly in C. It shows slides consisting of text and graphics contained in the ZIP file appended to the executable.

This is still work in progress, but I'm doing my best to separate working version (`main` branch) from progressing code (PRs).

![Lavender project logo](docs/lavender.png)

## Current features
* DOS, Windows, and diagnostic Linux targets
* [text-based script file format](docs/slides.md)
* graphics mode
  * CGA 640x200 monochrome under DOS
  * 24-bit RGB under Windows and Linux 
* display delays (animations)
  * millisecond resolution
* displaying text (with UTF-8 subset support)
  * under DOS, supports Czech, Polish, and Spanish diacritics, and 0x00-0x1F, 0x7F CP437 special characters
* displaying bitmap images
  * monochrome binary Portable BitMap (P4) files
  * XRGB8888 Windows Device Independent Bitmaps (BMP) under Windows and Linux
* drawing and filling rectangles
  * 16 colors on Windows and Linux
  * mapped 5 monochrome patterns on DOS
* MIDI Type 0 and [own mono format](docs/spk.md) music playback
  * PC Speaker, Yamaha OPL2, Roland MPU-401 UART under DOS
  * Windows MME API
  * FluidSynth under Linux
* script nesting
  * plain text, or encrypted using XOR or DES
* navigation using key presses and mouse clicks
* multiple language support (Czech, English, Polish)

## Building
Building requires x86_64 Linux with *CMake*, *GNU Make*, *GNU Binutils*, and `zip`.
Both Windows and Linux builds rely on *SDL2* and *SDL2_ttf* libraries.
Windows target is built using *MinGW-w64* for i686 or x86_64.
Linux build requires Fontconfig and FluidSynth.
MS-DOS builds require [GCC for IA-16](https://github.com/tkchia/gcc-ia16/) with [libi86](https://github.com/tkchia/libi86/).
Script encryption tools are written in *Python 3*.

If you have it all, you can configure the environment:
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

You can select user interface language (`ENU`, `CSY`, `PLK`) by adding the `-DLAV_LANG=` option.

After finishing configuration, navigate to the `build` directory and run `make bundle`. An executable called `sshow` should appear. You can modify the slideshow by editing files in the `data` directory.

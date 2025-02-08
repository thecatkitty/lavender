# Lavender ![Build status badge](https://github.com/thecatkitty/lavender/actions/workflows/build.yml/badge.svg?event=push)

A simple configurable slideshow program for DOS 2.0+ and Windows 95/NT4+ written mainly in C. It shows slides consisting of text and graphics contained in the ZIP file appended to the executable.

This is still work in progress, but I'm doing my best to separate working version (`main` branch) from progressing code (PRs).

![Lavender project logo](docs/lavender.png)

## Current features
* DOS, Windows, and diagnostic Linux targets
* [text-based script file format](docs/slides.md)
* graphics mode
  * CGA 640x200 monochrome under DOS
  * EGA 640x350 16-color under DOS using a loadable driver
  * 24-bit RGB under Windows and Linux with user-customizable content size
  * Windows DPI awareness and full screen mode support
* display delays (animations)
  * millisecond resolution
* displaying text (with UTF-8 subset support)
  * under DOS, supports Czech, Polish, and Spanish diacritics, and 0x00-0x1F, 0x7F CP437 special characters
* displaying bitmap images
  * monochrome and 16-color Windows Device Independent Bitmaps (BMP)
  * XRGB8888 Windows Device Independent Bitmaps (BMP) under Windows and Linux
* drawing and filling rectangles
  * 16 colors on Windows, Linux, and DOS with EGA
  * mapped 5 monochrome patterns on DOS with CGA
* MIDI Type 0 music playback
  * PC Speaker (with 3 simulated voices), Yamaha OPL2, Roland MPU-401 UART under DOS, with loadable driver support
  * Windows MME API
  * FluidSynth under Linux
* script nesting
  * plain text or encrypted
  * supported ciphers: XOR, DES, and TDES
  * decryption key stored locally, entered manually, or retrieved remotely (using request and confirmation code, QR and confirmation code, or over HTTP)
* navigation and user input using key presses and mouse clicks
* multiple language support (Czech, English, Polish)

## Building
Building requires *CMake*, *Python 3* and `zip` on all hosts.
On Linux hosts, *GNU Make* and *GNU Binutils* are also required.
Cross-compilation for Windows is done using [LLVM-MinGW](https://github.com/mstorsjo/llvm-mingw) for i586, x86_64, armv7, and aarch64.
Cross-compiled i586 Windows target links against [libunicows](https://libunicows.sourceforge.net/) and requires Internet Explorer 5 or newer.
Native compilation on Windows requires at least Microsoft Visual C++ 2010 with Windows SDK 7.1.
Linux builds rely on *SDL2*, *SDL2_ttf*, *cURL*, *Fontconfig*, *FluidSynth*, and *libblkid* libraries.
MS-DOS builds require [GCC for IA-16](https://github.com/tkchia/gcc-ia16/) with [libi86](https://github.com/tkchia/libi86/).

After acquiring all prerequisities, you can configure the Linux host environment, e. g.:
```sh
# Native (diagnostic) Linux build, either GCC or Clang is fine
cmake -S . -B build

# MS-DOS build, COM file (DOS 2+)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/DOS-GCC-IA16.cmake -DDOS_TARGET_COM=1

# MS-DOS build, EXE file (DOS 3+)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/DOS-GCC-IA16.cmake

# Windows build, IA-32 (Windows 95+)
CC=i686-w64-mingw32-gcc cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows

# Windows build, x64 (Windows XP+)
CC=x86_64-w64-mingw32-gcc cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows
```

You can select user interface language (`ENU`, `CSY`, `PLK`) by adding the `-DLAV_LANG=` option.

After finishing configuration, navigate to the `build` directory and run `make bundle`. An executable called `sshow` should appear. You can modify the slideshow by editing files in the `data` directory.

# Lavender

A simple configurable slideshow program for DOS 2.0+ written in 8086 assembly. It shows slides consisting of text and graphics contained in the ZIP file appended to the executable.

This is still work in progress, but I'm doing my best to separate working version (`main` branch) from progressing code (PRs).

## Current features
* CGA 640x200 monochrome video mode
* display delays
* displaying text (with UTF-8 subset support)
* displaying PBM bitmaps

## Format of `slides.txt`
Each line consists of these parts:
```
<delay> <type> <y> <x> <content>
```

`<delay>` is given in milliseconds.
`<type>` is either `T` (text) or `B` (bitmap).
`<y>` sets the vertical position (in lines for text, in pixels for graphics).
`<x>` sets the horizontal position (in columns for text, in pixels for graphics) or alignment (`<` for left, `^` for center, `>` for right). When a non-numeric value is provided, there's no whitespace after this field.
`<content>` sets the UTF-8 text or the bitmap file name.

The end of slides is marked with an empty line.

## Building
On Windows, MSYS2 has to be installed and present in `%PATH%`. 

Building on both Windows and Linux requires *GNU Make*, *GNU Binutils*, *Netwide Assembler* and `zip`. If you have it, just run:
```sh
make
```

If you want to change the output slideshow application file name (`sshow.com` is default) or the slideshow source directory, you can set `LAV_SSHOW` and `LAV_DATA` environmental variables respectively.

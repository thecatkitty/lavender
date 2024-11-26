# Format of `slides.txt`
The slideshow file consists of entries specifying various actions.
Each entry ends with CRLF.
Blank lines can be used for clarity.

- `<delay>` is a decimal integer of milliseconds that need to pass before action
- `<label>` is an arbitrary text up to 255 bytes, so is `<content>`
- `<alignment>` is either `<` for left, `^` for center, or `>` for right
- `<top>` and `<left>` set vertical and horizontal position in pixels
- `<width>` and `<height>` set width and height in pixels

## Label
```
:<label>
```

## Text
```
<delay> T <line> <column> <content>
- or -
<delay> T <line> <alignment><content>
```

`<line>` sets the vertical position in lines.
`<column>` sets the horizontal position in columns.

## Bitmap
```
<delay> B <top> <left> <name pattern>
- or -
<delay> B <top> <alignment><name pattern>
```

`<name pattern>` sets the bitmap file name, or file name pattern in case of pixel aspect ratio dependent images (in such case `<>` is a placeholder for encoding PAR).

## Rectangle
```
<delay> R|r <top> <left|alignment> <width> <height> <color>
```

`R` is rectangle border, `r` is rectangle fill. In border width, height, and position refer to the inner box.
`<color>` is `B` for black, `W` for white, or `G` for gray.
Apart from these, you can also use any of 16 basic color names defined by HTML specification (plus `cyan`).

## Play music
```
<delay> P <file name>
```
`<file name>` refers to the music file name.

## Wait for key press
```
<delay> K
```

Stores the scan code in the *Accumulator*.

## Define an active (clickable) area
```
<delay> A <line> <column> <width> <height> <tag>
```

When the area is clicked, `<tag>` is stored in the *Accumulator*.
In order to remove an area, set its `<tag>` to zero.

In order to remove all active areas:
```
<delay> A 0 0 0 0 0
```

## Environment query
```
<delay> ? <name>
```
Loads value of `<name>` into the *Accumulator*.

```
<delay> ? <name>:<value>
```
Updates `<name>` variable using the `<value>`.

### Defined variables
* gfx.colorful (R) - status of color support
  * 0 - absent
  * 1 - present
* gfx.title (W) - update the window title
  * 0 - does not apply
  * 1 - window title changed

Unknown queries return 65536.

## Unconditional jump
```
<delay> J <label>
```

## Conditional jump
```
<delay> = <value> <label>
```

Jumps to `<label>` if the *Accumulator* is equal to the `<value>`.

## Script call
```
<delay> ! <script>
- or -
<delay> ! <script> <method> <crc32> <parameter> <data>
```

Calls a script file named `<script>` as plain text or using some encryption method.
`<method>` is 0 for plain text, 1 for XOR48, 2 for DES.
`<crc32>` is hexadecimal representation of plaintext ZIP CRC32.
`<parameter>` is a decimal integer.
`<data>` is up to 63 characters.

### Encryption parameter values
* method 0 - plain text
* method 1 - XOR48
  * parameter 0 - hexadecimal key stored in `<data>`
  * parameter 1 - hexadecimal key entered by the user
  * parameter 8 - key split between `<data>` and a passcode entered by the user
  * parameter 9 - key split between the volume ID and a passcode entered by the user
* method 2 - DES, method 3 - TDES (Keying option 2)
  * parameter 0 - hexadecimal key stored in `<data>`
  * parameter 1 - hexadecimal key entered by the user
  * parameter 8 - 25-character CD key entered by the user
  * parameter 9 - remote key delivery (25-character access code with interactive unlock)

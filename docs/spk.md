# Format of the PC speaker music file
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

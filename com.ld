SECTIONS
{
  . = 0x0100;
  .com :
  {
    *(.text);
    *(.data);
  }
  .bss (NOLOAD) :
  {
    *(.bss*);
    *(.reloc*);
  }
}

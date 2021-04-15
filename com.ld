SECTIONS
{
  . = 0x0100;
  .com :
  {
    *(.init);
    *(.text);
    *(.data);
  }
  .bss (NOLOAD) :
  {
    *(.bss*);
    *(.reloc*);
  }
}

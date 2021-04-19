SECTIONS
{
  . = 0x0100;
  .com :
  {
    *(.init);
    *(.text);
    *(.data);
  }
  _ext_start = .;
  . = 0x3000;
  .bss (NOLOAD) :
  {
    *(.bss*);
    *(.reloc*);
  }
}

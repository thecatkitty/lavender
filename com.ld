SECTIONS
{
  . = 0x0100;
  .com :
  {
    *(.init);
    *(.text);
    *(.data);
  }
  ArchiveStart = .;
  . = 0x3000;
  ArchiveEnd = .;
  .bss (NOLOAD) :
  {
    *(.bss*);
    *(.reloc*);
  }
}

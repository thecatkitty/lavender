MEMORY
{
  FILE : ORIGIN = 0x0100, LENGTH = 12K
  REST : ORIGIN = 0x3000, LENGTH = 4K
}

SECTIONS
{
  .com :
  {
    *(.init);
    *(.text);
    *(.data);
    ArchiveStart = .;
  } > FILE
  .bss (NOLOAD) :
  {
    ArchiveEnd = .;
    *(.bss*);
    StackTop = .;
    *(.reloc*);
  } > REST
  StackBottom = ORIGIN(REST) + LENGTH(REST) - 2;
}

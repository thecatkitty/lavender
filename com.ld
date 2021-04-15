SECTIONS
{
  . = 0x0100;
  .text :
  {
    *(.text);
    *(.data);
  }
  .bss (NOLOAD) :
  {
    *(.bss*);
  }
}

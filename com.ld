/* Linker script for DOS executables with combined data and text segment,
   with startup files, with default libraries, and without
   DPMI support.

   Generated from dos-mt.ld.in.  */

OUTPUT_FORMAT(binary)
ENTRY(_start)
INPUT(-l:crtbegin.o -l:crtend.o -l:dos-t-c0.o)
GROUP(-lc -lgcc -ldos-t -lm)

MEMORY
  {
    FILE : ORIGIN = 0x0100, LENGTH = 0xDF00 /* 55.75K */
    REST : ORIGIN = 0xE000, LENGTH = 0x1D00 /*  7.25K */
  }

SECTIONS
  {
    . = SEGMENT_START ("text-segment", 0x100);
    /* Target text sections.  */
    .text : {
		__stext = .;
		*(.startupA)
		*(.head) *(.head.*)
		*(.startupB)
		*(.startupC)
		*(.msdos_init) *(.msdos_init.*)
		*(.msdos_initA) *(.msdos_initA.*)
		*(.startupD)
		*(.msdos_initB) *(.msdos_initB.*)
		*(.startupE)
		*(.init)
		*(.startupF)
		*(.exitA)
		*(.fini)
		*(.exitB)
		*(.text) *(.text.*)
		__etext = .;
	} > FILE
	__ltext = __etext - __stext;

    /* Target data sections.  */
    .data : {
		__sdata = .;

		/* Build lists of constructors and destructors.  */
		KEEP (*crtbegin*.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))

		KEEP (*crtbegin*.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))

		*(.msdos_init_rodata) *(.msdos_init_rodata.*)
		*(.rodata) *(.rodata.*)
		*(.autofloat_stdio.__ia16_have_printf_float.dprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._dprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vdprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vdprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.sprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._sprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.fprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._fprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.printf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._printf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.snprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._snprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.asprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._asprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.asnprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._asnprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vfprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vfprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vsprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vsprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vsnprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vsnprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vasprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vasprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_printf_float.vasnprintf.v2)
		*(.autofloat_stdio.__ia16_have_printf_float._vasnprintf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.sscanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._sscanf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.fscanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._fscanf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.scanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._scanf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.vscanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._vscanf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.vfscanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._vfscanf_r.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float.vsscanf.v2)
		*(.autofloat_stdio.__ia16_have_scanf_float._vsscanf_r.v2)
		. = .;
		*(.data) *(.data.*)
		*(.gcc_except_table)

		__serrf = .;
		*(.errf)
		__serrm = .;
		*(.errm)

		__edata = .;
	} > FILE

    .bss (NOLOAD) : {
		 __sbss = .;
                *(.bss) *(.bss.*)
                *(COMMON)
                __ebss = .;

                /* Minimum address allowed for sbrk() to use.  */
                __heap_end_minimum = ALIGN(8);
	} > REST

	__ldata = __edata - __sdata;
	__lbss0 = __ebss - __sbss;
	__lbss1 = __lbss0 + 1;
	__lbss = __lbss1 / 2;

    ASSERT(. <= 0xfd00, "Error: too large for a .com file.")

    /* Used by the memory resizing and DPMI initialization code in
       dos-models-crt0.S.  */
    __msdos_initial_alloc_paras = 0x1000;

    /DISCARD/ : { *(.*) }
  }

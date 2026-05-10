/* Linker script for PbOS kernel. */

ENTRY(kernel_main)

OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x80000000;

	KI_KCODE_BEGIN = .;
	.text :
	{
		*(.text)
		*(.text.*)
	}
	KI_KCODE_END = .;
	.data :
	{
		*(.data)
		*(.data.*)
	}
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	}
	.strtab :
	{
		*(.strtab)
	}
	.shstrtab :
	{
		*(.shstrtab)
	}
	.bss :
	{
		*(.bss)
	}

	.init_array :
	{
		KI_CTORS_BEGIN = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .init_array))
	}
	KI_CTORS_END = .;

	.fini_array :
	{
		KI_DTORS_BEGIN = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .fini_array))
	}
	KI_DTORS_END = .;

	KI_EXPORTED_SYMBOLS_BEGIN = .;
	.EXPORTED_SYMBOLS :
	{
		*(EXPORTED_SYMBOLS)
	}
	KI_EXPORTED_SYMBOLS_END = .;
}

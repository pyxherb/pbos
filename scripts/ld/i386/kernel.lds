/* Linker script for PbOS kernel. */

ENTRY(_start)

OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x80000000;

	KN_KCODE_BEGIN = .;
	.text :
	{
		*(.text)
	}
	KN_KCODE_END = .;
	.data :
	{
		*(.data)
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

	KN_EXPORTED_SYMBOLS_BEGIN = .;
	.EXPORTED_SYMBOLS :
	{
		*(EXPORTED_SYMBOLS)
	}
	KN_EXPORTED_SYMBOLS_END = .;
}

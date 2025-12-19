/* Linker script for PbOS kernel. */

ENTRY(_start)

OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x00400000;

	. = ALIGN(0x1000);
	.text :
	{
		*(.text)
	}
	. = ALIGN(0x1000);
	.data :
	{
		*(.data)
	}
	. = ALIGN(0x1000);
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	}
	. = ALIGN(0x1000);
	.strtab :
	{
		*(.strtab)
	}
	. = ALIGN(0x1000);
	.shstrtab :
	{
		*(.shstrtab)
	}
	. = ALIGN(0x1000);
	.bss :
	{
		*(.bss)
	}
}

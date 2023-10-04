/* Linker script for OicOS kernel. */

ENTRY(_start)

OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x00400000;

	.text :
	{
		*(.text)
	}
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
}

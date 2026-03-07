/* Linker script for PbOS kernel. */

ENTRY(_start)

OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	. = 0x0000000000400000;

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

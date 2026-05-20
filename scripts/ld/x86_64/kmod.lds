/* Linker script for PbOS kernel modules. */

OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	.text :
	{
		*(.text)
		*(.text.*)
	}

	. = ALIGN(CONSTANT(MAXPAGESIZE));

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

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.dynamic :
	{
		*(.dynamic)
		*(.dynamic.*)
	}
	.dynsym :
	{
		*(.dynsym)
		*(.dynsym.*)
	}
	.rela :
	{
		*(.rela)
		*(.rela.*)
	}
	.dynstr :
	{
		*(.dynstr)
		*(.dynstr.*)
	}
	.hash :
	{
		*(.hash)
		*(.hash.*)
	}
	.gnu.hash :
	{
		*(.gnu.hash)
		*(.gnu.hash.*)
	}

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data :
	{
		*(.data)
		*(.data.*)
	}

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.init_array :
	{
		HIDDEN(KXI_CTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .init_array))
		HIDDEN(KXI_CTORS_END = .);
	}

	.fini_array :
	{
		HIDDEN(KXI_DTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .fini_array))
		HIDDEN(KXI_DTORS_END = .);
	}

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.bss :
	{
		*(.bss)
	}
}

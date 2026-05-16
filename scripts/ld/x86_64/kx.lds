/* Linker script for PbOS kernel. */

ENTRY(kmain)

OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	.text :
	{
		*(.text)
		*(.text.*)
	} :text

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	} :rodata
	.strtab :
	{
		*(.strtab)
	} :rodata
	.shstrtab :
	{
		*(.shstrtab)
	} :rodata

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.dynamic :
	{
		*(.dynamic)
		*(.dynamic.*)
	} :dynsym
	.dynsym :
	{
		*(.dynsym)
		*(.dynsym.*)
	} :dynsym :dynsym_rodata
	.rela :
	{
		*(.rela)
		*(.rela.*)
	} :dynsym
	.dynstr :
	{
		*(.dynstr)
		*(.dynstr.*)
	} :dynsym :dynstr_rodata
	.hash :
	{
		*(.hash)
		*(.hash.*)
	} :dynsym
	.gnu.hash :
	{
		*(.gnu.hash)
		*(.gnu.hash.*)
	} :dynsym

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data :
	{
		*(.data)
		*(.data.*)
	} :data

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.init_array :
	{
		HIDDEN(KXI_CTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .init_array))
		HIDDEN(KXI_CTORS_END = .);
	} :data

	.fini_array :
	{
		HIDDEN(KXI_DTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .fini_array))
		HIDDEN(KXI_DTORS_END = .);
	} :data

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.bss :
	{
		*(.bss)
	} :data
}

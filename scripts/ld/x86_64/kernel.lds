/* Linker script for PbOS kernel. */

ENTRY(kmain)

OUTPUT_FORMAT(elf64-x86-64)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
	dynsym PT_DYNAMIC;
	dynsym_rodata PT_LOAD;
	dynstr_rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
	. = 0xffffffff80000000;
	.limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	HIDDEN(KI_KCODE_BEGIN = .);
	.text :
	{
		*(.text)
		*(.text.*)
	} :text
	HIDDEN(KI_KCODE_END = .);

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
	.rela :
	{
		*(.rela)
		*(.rela.*)
	} :dynsym
	.dynsym :
	{
		HIDDEN(KI_EXPORTED_SYMBOLS_BEGIN = .);
		*(.dynsym)
		*(.dynsym.*)
		HIDDEN(KI_EXPORTED_SYMBOLS_END = .);
	} :dynsym :dynsym_rodata
	.dynstr :
	{
		HIDDEN(KI_EXPORTED_SYMBOL_NAMES_BEGIN = .);
		*(.dynstr)
		*(.dynstr.*)
		HIDDEN(KI_EXPORTED_SYMBOL_NAMES_END = .);
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
		HIDDEN(KI_CTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .init_array))
		HIDDEN(KI_CTORS_END = .);
	} :data

	.fini_array :
	{
		HIDDEN(KI_DTORS_BEGIN = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .fini_array))
		HIDDEN(KI_DTORS_END = .);
	} :data

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.bss :
	{
		*(.bss)
	} :data
}

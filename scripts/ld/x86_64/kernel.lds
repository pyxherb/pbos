/* Linker script for PbOS kernel. */

ENTRY(kmain)

OUTPUT_FORMAT(elf64-x86-64)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
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

	KI_KCODE_BEGIN = .;
	.text :
	{
		*(.text)
		*(.text.*)
	} :text
	KI_KCODE_END = .;

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

	.data :
	{
		*(.data)
		*(.data.*)
	} :data

	.init_array :
	{
		KI_CTORS_BEGIN = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .init_array))
	} :data
	KI_CTORS_END = .;

	.fini_array :
	{
		KI_DTORS_BEGIN = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .fini_array))
	} :data
	KI_DTORS_END = .;

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.bss :
	{
		*(.bss)
	} :data
}

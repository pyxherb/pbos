/* Linker script for the boot loader. */

ENTRY(_start)

OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0x00100000;

    .text : {
		*(.boot)
        *(.text)
    }
    .data : {
        *(.data)
    }
    .rodata : {
		*(.rodata)
		*(.rodata.*)
    }
    .strtab : {
		*(.strtab)
    }
	.shstrtab : {
        *(.shstrtab)
	}
	.bss : {
        *(.bss)
	}

	.comment : {
		*(.comment)
	}
	.symtab : {
		*(.symtab)
	}
	.debug : {
		*(.debug_*)
	}
}

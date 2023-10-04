#include <arch/i386/asm/mm.h>

#define MB2_HDR_MAGIC 0xe85250d6
#define MB2_TARGET_ARCH 0

.section .boot
// MULTIBOOT START
mb2_hdr:
.long MB2_HDR_MAGIC // Magic
.long MB2_TARGET_ARCH // Target Machine
.long (mb2_hdr_end-mb2_hdr) // Header Length
.long -(MB2_HDR_MAGIC + MB2_TARGET_ARCH + (mb2_hdr_end - mb2_hdr)) // Checksum

hdraddr_tag:
.short 2 // Type
.short 0 // Flags
.long (hdraddr_tag_end - hdraddr_tag) // Size
.long mb2_hdr // Header Address
.long _start // Load Address
.long 0 // Load End
.long (KERNEL_PBASE + KERNEL_MAX_SIZE) // BSS End
.align 8
hdraddr_tag_end:

entry_tag:
.short 3 // Type
.short 0 // Flags
.long (entry_tag_end - entry_tag) // Size
.long _start // Entry
.align 8
entry_tag_end:


end_tag:
.short 0 // Type
.short 0 // Flags
.long (end_tag_end-end_tag) // Size
.align 8
end_tag_end:

mb2_hdr_end:
// MULTIBOOT END.

// Selectors
#define CODE_SELECTOR SELECTOR(0, 0, 1)
#define DATA_SELECTOR SELECTOR(0, 0, 2)

.text
.global _start
_start:
	// Get Multiboot2 boot information.
	movl %ebx, (boot_p_info)
	movl %eax, (boot_mb_magic)

	// Disable NMI.
	inb $0x70, %al
	orb $0x80, %al
	outb %al, $0x70

	// Initialize the stack. The stack area is 0x00500-0x07bff
	movl $0x07bff, %ebp
	movl %ebp ,%esp

	// Switch GDT.
	lgdt gdtr

	// Load the code selector to CS register.
	ljmpl $CODE_SELECTOR,$post_early_init

post_early_init:
	movw $DATA_SELECTOR, %ax
	movw %ax, %ds
	movw %ax, %ss

	calll boot_main

	movl $(0x80c06000 + 0x0000a000), %esp
	xorl %ebp, %ebp

	jmp *(boot_kentry)

.data
// GDT
gdt:
	null_desc: GDTDESC(0, 0, 0, 0)
	code_desc: GDTDESC(0x00000000, 0xfffff, GDT_ACCESS_BYTE(1, 0, 1, 1, 1, 0, 0), GDT_FLAGS(1, 1))
    data_desc: GDTDESC(0x00000000, 0xfffff, GDT_ACCESS_BYTE(1, 0, 1, 0, 0, 1, 0), GDT_FLAGS(1, 1))
gdt_end:

gdtr:
	gdtr_limit: .short gdt_end-gdt-1
	gdtr_base: .long gdt

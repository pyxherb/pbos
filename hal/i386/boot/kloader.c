#include "kloader.h"
#include <arch/i386/misc.h>
#include <arch/i386/mlayout.h>
#include <hal/i386/display/vga.h>
#include <pbos/fmt/elf.h>
#include <pbos/fmt/pbkim.h>
#include <string.h>
#include "error.h"
#include "misc.h"

PB_NORETURN void (*boot_kentry)();

#define PHDR(base, sz_entry, i) ((Elf32_Phdr *)(((const void *)base) + sz_entry * i))

bool boot_load_pbkim() {
	pbkim_ihdr_t *ih = (pbkim_ihdr_t *)KERNEL_IMAGE_BASE;
	{
		if (ih->magic[0] != OICKIM_MAGIC_0 ||
			ih->magic[1] != OICKIM_MAGIC_1 ||
			ih->magic[2] != OICKIM_MAGIC_2 ||
			ih->magic[3] != OICKIM_MAGIC_3) {
			boot_seterr("Bad OICKIM magic number");
			return false;
		}

		if (ih->machine != OICKIM_MACHINE_I386) {
			boot_seterr("Unacceptable OICKIM machine");
			return false;
		}
	}

	void *pbkim_ptr = &(ih[1]);
	Elf32_Ehdr *ehdr = pbkim_ptr;
	{
		if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
			ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
			boot_seterr("Bad ELF image");
			return false;
		}

		if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
			boot_seterr("Bad ELF version");
			return false;
		}

		if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
			boot_seterr("Unacceptable ELF data encoding");
			return false;
		}

		if (ehdr->e_ident[EI_OSABI] != ELFOSABI_NONE) {
			boot_seterr("Unacceptable ELF ABI");
			return false;
		}

		if (ehdr->e_type != ET_EXEC) {
			boot_seterr("Unacceptable ELF image type");
			return false;
		}

		if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
			boot_seterr("Unacceptable ELF class");
			return false;
		}

		if (ehdr->e_entry < KERNEL_PBASE || ehdr->e_entry >= (KERNEL_VBASE + KERNEL_SIZE)) {
			boot_seterr("ELF load base is too low");
			return false;
		}
	}

	// Load for each segment.
	Elf32_Half phdr_num = ehdr->e_phnum;
	Elf32_Phdr *phdr = pbkim_ptr + ehdr->e_phoff;

	for (Elf32_Half i = 0; i < phdr_num; ++i) {
		// Current program header.
		Elf32_Phdr *ph = PHDR(phdr, ehdr->e_phentsize, i);

		if (ph->p_type != PT_LOAD)
			continue;

		if (ph->p_vaddr < KERNEL_VBASE) {
			boot_seterr("Kernel load base is too low");
			return false;
		}

		if ((((uint64_t)ph->p_vaddr) + ph->p_memsz) >= (KERNEL_VBASE + KERNEL_SIZE)) {
			boot_seterr("Kernel load max is too high");
			return false;
		}

		if (ph->p_filesz > ph->p_memsz) {
			boot_seterr("Invalid segment size combination");
			return false;
		}

		memcpy((void *)(ph->p_vaddr), pbkim_ptr + ph->p_offset, ph->p_filesz);

		if (ph->p_filesz < ph->p_memsz)
			memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
	}

	boot_kentry = (void *)ehdr->e_entry;

	return true;
}

#include <arch/i386/mlayout.h>
#include <arch/i386/paging.h>
#include <pbos/fmt/elf.h>
#include <pbos/fs/file.h>
#include <pbos/km/exec.h>
#include <pbos/km/logger.h>
#include <string.h>
#include <pbos/hal/irq.hh>

km_result_t kn_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp);
km_result_t kn_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp);

km_binldr_t kn_binldr_elf = {
	.load_exec = kn_elf_load_exec,
	.load_mod = kn_elf_load_mod
};

km_result_t kn_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp) {
	km_result_t result;
	size_t off = 0, bytes_read;

	ps_tcb_t *tcb = ps_alloc_tcb(proc);
	if (!tcb)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	// Allocate stack for main thread.
	if (KM_FAILED(result = ps_thread_alloc_stack(tcb, 0x200000)))
		return result;

	if (KM_FAILED(result = ps_thread_alloc_kernel_stack(tcb, 0x2000)))
		return result;

	Elf32_Ehdr ehdr;
	if (KM_FAILED(result = fs_read(file_fp, &ehdr, sizeof(ehdr), off, &bytes_read))) {
		// TODO: free allocated resources here.
		return result;
	}
	off += bytes_read;

	{
		if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 || ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			ehdr.e_ident[EI_MAG3] != ELFMAG3)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		if (ehdr.e_ident[EI_VERSION] != EV_CURRENT)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		if (ehdr.e_ident[EI_OSABI] != ELFOSABI_NONE)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		if (ehdr.e_type != ET_EXEC)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		if (ehdr.e_ident[EI_CLASS] != ELFCLASS32)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);
	}

	Elf32_Half phdr_num = ehdr.e_phnum;
	off += bytes_read;

	for (Elf32_Half i = 0; i < phdr_num; ++i) {
		// Current program header.
		Elf32_Phdr ph;
		if (!KM_SUCCEEDED(result = fs_read(file_fp, &ph, sizeof(ph), ehdr.e_phoff + ehdr.e_phentsize * i, &bytes_read))) {
			// TODO: free allocated resources here.
			return result;
		}

		if (ph.p_type != PT_LOAD)
			continue;

		if ((((uint64_t)ph.p_vaddr) + ph.p_memsz) >= KERNEL_VBASE) {
			return KM_MAKEERROR(KM_RESULT_INVALID_ADDR);
		}

		if (ph.p_filesz > ph.p_memsz)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		char *vaddr = (char *)PGFLOOR(ph.p_vaddr);

		off = ph.p_offset;

		io::irq_disable_lock irq_disable_lock;
		mm_switch_context(ps_mm_context_of(proc));
		{
			// Allocate pages for current segment.
			for (Elf32_Word j = 0; j < ph.p_memsz; j += PAGESIZE) {
				if (!mm_getmap(ps_mm_context_of(proc), vaddr + PAGESIZE * j, NULL)) {
					void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
					mm_mmap(ps_mm_context_of(proc), vaddr + PAGESIZE * j, paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_EXEC | PAGE_USER, 0);
				}
			}

			// Read the whole segment into the memory.
			memset((void *)ph.p_vaddr, 0, ph.p_memsz);
			if (!KM_SUCCEEDED(result = fs_read(file_fp, (void *)ph.p_vaddr, ph.p_filesz, ph.p_offset, &bytes_read))) {
				// TODO: free allocated resources here.
				return result;
			}
			off += bytes_read;

			// Mark the pages as executable.
			mm_chpgmod(ps_mm_context_of(proc), vaddr, ph.p_memsz, PAGE_EXEC);
		}
		mm_switch_context(mm_kernel_context);
	}

	// Set entry of main thread.
	ps_user_thread_init(tcb);
	ps_thread_set_entry(tcb, (void *)ehdr.e_entry);

	ps_add_thread(proc, tcb);

	return KM_RESULT_OK;
}

km_result_t kn_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp) {}

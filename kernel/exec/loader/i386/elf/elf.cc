#include <arch/i386/mlayout.h>
#include <arch/i386/paging.h>
#include <pbos/fmt/elf.h>
#include <pbos/fs/file.h>
#include <pbos/km/exec.h>
#include <pbos/km/logger.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>

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

	// TODO: Add cleanup guard.

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
		if (KM_FAILED(result = fs_read(file_fp, &ph, sizeof(ph), ehdr.e_phoff + ehdr.e_phentsize * i, &bytes_read))) {
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

		char *vaddr = (char *)ph.p_vaddr;

		if (((uintptr_t)vaddr) % PAGESIZE)
			return KM_MAKEERROR(KM_RESULT_INVALID_FMT);

		off = ph.p_offset;

		mm_context_t *prev_context = mm_get_cur_context();
		{
			void *tmp_pgvaddr;

			tmp_pgvaddr = mm_kvmalloc(prev_context, PAGESIZE, PAGE_MAPPED | PAGE_WRITE, 0);
			if (!tmp_pgvaddr)
				return KM_MAKEERROR(KM_RESULT_NO_MEM);

			{
				io::irq_disable_lock lock;
				kfxx::scope_guard restore_context_guard([prev_context, tmp_pgvaddr]() noexcept {
					mm_vmfree(prev_context, tmp_pgvaddr, PAGESIZE);
				});

				// Allocate pages for current segment.
				for (size_t j = 0; j < ph.p_memsz; j += PAGESIZE) {
					void *paddr;
					if (!(paddr = mm_getmap(ps_mm_context_of(proc), vaddr + j, NULL))) {
						paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
						klog_printf("paddr: %p\n", paddr);
						if (!paddr)
							return KM_MAKEERROR(KM_RESULT_NO_MEM);
						if(paddr == (void*)0x05025000) {
							asm volatile("xchg %bx, %bx");
							klog_puts("");
						}
					}

					if (KM_FAILED(result = mm_mmap(ps_mm_context_of(proc), vaddr + j, paddr, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_EXEC | PAGE_USER, 0)))
						return result;
					kd_assert(mm_getmap(ps_mm_context_of(proc), vaddr + j, NULL) == paddr);
					if (KM_FAILED(
							result = mm_mmap(
								prev_context,
								tmp_pgvaddr,
								paddr,
								PAGESIZE,
								PAGE_WRITE | PAGE_MAPPED,
								0)))
						return result;
					memset((void *)tmp_pgvaddr, 0, PAGESIZE);
					if (j < ph.p_filesz) {
						if (KM_FAILED(result = fs_read(
											  file_fp,
											  tmp_pgvaddr,
											  PBOS_MIN(ph.p_filesz - j, PAGESIZE),
											  off,
											  &bytes_read))) {
							// TODO: free allocated resources here.
							return result;
						}
						off += bytes_read;
					}
				}
			}

			// Mark the pages as executable.
			// mm_set_page_access(ps_mm_context_of(proc), vaddr, ph.p_memsz, PAGE_MAPPED | PAGE_EXEC);
		}
	}

	// Set entry of main thread.
	ps_user_thread_init(tcb);
	ps_thread_set_entry(tcb, (void *)ehdr.e_entry);

	ps_add_thread(proc, tcb);

	return KM_RESULT_OK;
}

km_result_t kn_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp) {}

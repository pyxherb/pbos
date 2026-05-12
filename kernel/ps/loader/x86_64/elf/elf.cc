#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <elf.h>
#include <pbos/ps/exec.h>
#include <pbos/fs/file.h>
#include <pbos/kd/logger.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/list.hh>
#include <pbos/kfxx/scope_guard.hh>

km_result_t ki_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp);
km_result_t ki_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp);
km_result_t ki_elf_load_kernel_mod(fs_fcb_t *file_fp);

km_binldr_ops_t ki_binldr_elf = {
	.load_exec = ki_elf_load_exec,
	.load_mod = ki_elf_load_mod,
	.load_kmod = ki_elf_load_kernel_mod
};

km_result_t ki_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp) {
	km_result_t result;
	size_t off = 0, bytes_read;

	ps_tcb_t *tcb = ps_alloc_tcb(proc);
	if (!tcb)
		return KM_RESULT_NO_MEM;

	// Allocate stack for main thread.
	if (KM_FAILED(result = ps_thread_alloc_stack(tcb, 0x200000)))
		return result;

	// TODO: Add cleanup guard.

	if (KM_FAILED(result = ps_thread_alloc_kernel_stack(tcb, 0x2000)))
		return result;

	Elf64_Ehdr ehdr;
	if (KM_FAILED(result = fs_read(file_fp, &ehdr, sizeof(ehdr), off, &bytes_read))) {
		// TODO: free allocated resources here.
		return result;
	}
	off += bytes_read;

	{
		if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 || ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			ehdr.e_ident[EI_MAG3] != ELFMAG3)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_VERSION] != EV_CURRENT)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_OSABI] != ELFOSABI_NONE)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_type != ET_EXEC)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_CLASS] != ELFCLASS64)
			return KM_RESULT_MALFORMED;
	}

	Elf64_Half phdr_num = ehdr.e_phnum;
	off += bytes_read;

	for (Elf64_Half i = 0; i < phdr_num; ++i) {
		// Current program header.
		Elf64_Phdr ph;
		if (KM_FAILED(result = fs_read(file_fp, &ph, sizeof(ph), ehdr.e_phoff + ehdr.e_phentsize * i, &bytes_read))) {
			// TODO: free allocated resources here.
			return result;
		}

		if (ph.p_type != PT_LOAD)
			continue;

		if ((((uint64_t)ph.p_vaddr) + ph.p_memsz) >= KERNEL_VBASE) {
			return KM_RESULT_INVALID_ADDR;
		}

		if (ph.p_filesz > ph.p_memsz)
			return KM_RESULT_MALFORMED;

		char *vaddr = (char *)ph.p_vaddr;

		if (((uintptr_t)vaddr) % PAGESIZE)
			return KM_RESULT_MALFORMED;

		off = ph.p_offset;

		mm_context_t *cur_context = mm_get_cur_context(),
					 *target_context = ps_mm_context_of(proc);
		{
			void *tmp_pgvaddr;

			tmp_pgvaddr = mm_kvmalloc(cur_context, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_WRITE, 0);
			if (!tmp_pgvaddr)
				return KM_RESULT_NO_MEM;

			mm_pgaccess_t pgaccess = MM_PAGE_MAPPED | MM_PAGE_USER;

			if (ph.p_flags & PF_R)
				pgaccess |= MM_PAGE_READ;
			if (ph.p_flags & PF_W)
				pgaccess |= MM_PAGE_WRITE;
			if (ph.p_flags & PF_X)
				pgaccess |= MM_PAGE_EXEC;

			{
				kfxx::scope_guard unmap_tmp_pgvaddr_guard([cur_context, tmp_pgvaddr]() noexcept {
					mm_vmfree(cur_context, tmp_pgvaddr, PAGESIZE);
				});

				// Allocate pages for current segment.
				for (size_t j = 0; j < ph.p_memsz; j += PAGESIZE) {
					void *paddr;
					if (!(paddr = mm_getmap(target_context, vaddr + j, nullptr))) {
						paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
						klog_printf("vaddr: %p, paddr: %p\n", vaddr + j, paddr);
						if (!paddr)
							return KM_RESULT_NO_MEM;
					}

					if (KM_FAILED(result = mm_mmap(target_context, vaddr + j, paddr, PAGESIZE, pgaccess, 0)))
						return result;
					if (KM_FAILED(
							result = mm_mmap(
								cur_context,
								tmp_pgvaddr,
								paddr,
								PAGESIZE,
								MM_PAGE_WRITE | MM_PAGE_MAPPED,
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
					if (j)
						km_unwrap_result(mm_merge_mapped_area(target_context, vaddr, vaddr + j));
				}
			}

			// Mark the pages as executable.
			// mm_set_page_access(target_context, vaddr, ph.p_memsz, MM_PAGE_MAPPED | MM_PAGE_EXEC);
		}
	}

	// Set entry of main thread.
	ps_user_thread_init(tcb);
	ps_thread_set_entry(tcb, (void *)ehdr.e_entry);

	ps_add_thread(proc, tcb);

	return KM_RESULT_OK;
}

km_result_t ki_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp) {}

km_result_t ki_elf_load_kernel_mod(fs_fcb_t *file_fp) {
	km_result_t result;
	size_t off = 0, bytes_read;

	Elf64_Ehdr ehdr;
	if (KM_FAILED(result = fs_read(file_fp, &ehdr, sizeof(ehdr), off, &bytes_read))) {
		// TODO: free allocated resources here.
		return result;
	}
	off += bytes_read;

	{
		if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 || ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			ehdr.e_ident[EI_MAG3] != ELFMAG3)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_VERSION] != EV_CURRENT)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_OSABI] != ELFOSABI_NONE)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_type != ET_REL)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_CLASS] != ELFCLASS64)
			return KM_RESULT_MALFORMED;
	}

	Elf64_Half shdr_num = ehdr.e_shnum;
	off += bytes_read;

	for (Elf64_Half i = 0; i < shdr_num; ++i) {
		Elf64_Shdr sh;
		if (KM_FAILED(result = fs_read(file_fp, &sh, sizeof(sh), ehdr.e_shoff + ehdr.e_shentsize * i, &bytes_read))) {
			// TODO: free allocated resources here.
			return result;
		}

		if (sh.sh_flags & SHF_ALLOC)
			continue;

		if(sh.sh_addralign % PAGESIZE)
			return KM_RESULT_MALFORMED;

		mm_pgaccess_t pgaccess = MM_PAGE_MAPPED | MM_PAGE_READ;

		if (sh.sh_flags & SHF_WRITE)
			pgaccess |= MM_PAGE_WRITE;
		if (sh.sh_flags & SHF_EXECINSTR)
			pgaccess |= MM_PAGE_EXEC;

		switch (sh.sh_type) {
			case SHT_PROGBITS: {
				break;
			}
			case SHT_NOBITS: {
				break;
			}
			case SHT_REL: {
				break;
			}
		}
	}

	return KM_RESULT_OK;
}

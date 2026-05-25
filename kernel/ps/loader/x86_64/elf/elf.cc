#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <elf.h>
#include <pbos/fs/file.h>
#include <pbos/kd/logger.h>
#include <pbos/ps/exec.h>
#include <pbos/ps/kmod.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/dynarray.hh>
#include <pbos/kfxx/list.hh>
#include <pbos/kfxx/scope_guard.hh>

km_result_t ki_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp);
km_result_t ki_elf_load_mod(ps_pcb_t *proc, fs_fcb_t *file_fp);
km_result_t ki_elf_load_kmod(ps_kmod_t *kmod, fs_fcb_t *file_fp);

km_binldr_ops_t ki_binldr_elf = {
	.load_exec = ki_elf_load_exec,
	.load_mod = ki_elf_load_mod,
	.load_kmod = ki_elf_load_kmod
};

km_result_t ki_elf_load_exec(ps_pcb_t *proc, fs_fcb_t *file_fp) {
	klog_printf("test\n");
	km_result_t result;
	size_t off = 0, bytes_read;
	const size_t page_size = mm_get_page_size();

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
			return KM_RESULT_MALFORMED;
		}

		if (ph.p_filesz > ph.p_memsz)
			return KM_RESULT_MALFORMED;

		char *vaddr = (char *)ph.p_vaddr;

		if (((uintptr_t)vaddr) % page_size)
			return KM_RESULT_MALFORMED;

		off = ph.p_offset;

		mm_context_t *cur_context = mm_get_cur_context(),
					 *target_context = ps_mm_context_of(proc);
		{
			void *tmp_pgvaddr;

			tmp_pgvaddr = mm_kvmalloc(cur_context, page_size, MM_PAGE_MAPPED | MM_PAGE_WRITE, 0);
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
				kfxx::scope_guard unmap_tmp_pgvaddr_guard([cur_context, tmp_pgvaddr, page_size]() noexcept {
					mm_vmfree(cur_context, tmp_pgvaddr, page_size);
				});

				// Allocate pages for current segment.
				for (size_t j = 0; j < ph.p_memsz; j += page_size) {
					void *paddr;
					if (!(paddr = mm_getmap(target_context, vaddr + j, nullptr))) {
						paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
						if (!paddr)
							return KM_RESULT_NO_MEM;
					}

					{
						kfxx::scope_guard free_paddr_guard([paddr]() noexcept {
							mm_unpin_page(paddr);
						});

						if (KM_FAILED(
								result = mm_mmap(
									cur_context,
									tmp_pgvaddr,
									paddr,
									page_size,
									MM_PAGE_WRITE | MM_PAGE_MAPPED,
									MMAP_NO_INC_RC)))
							return result;
						free_paddr_guard.release();
					}

					if (KM_FAILED(result = mm_mmap(target_context, vaddr + j, paddr, page_size, pgaccess, MMAP_NO_INC_RC)))
						return result;
					memset((void *)tmp_pgvaddr, 0, page_size);
					if (j < ph.p_filesz) {
						if (KM_FAILED(result = fs_read(
										  file_fp,
										  tmp_pgvaddr,
										  PBOS_MIN(ph.p_filesz - j, page_size),
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

km_result_t ki_elf_load_kmod(ps_kmod_t *kmod, fs_fcb_t *file_fp) {
	mm_context_t *mm_context = mm_get_cur_context();
	km_result_t result;
	size_t off = 0, bytes_read;
	const size_t page_size = mm_get_page_size();

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

		if (ehdr.e_type != ET_DYN)
			return KM_RESULT_MALFORMED;

		if (ehdr.e_ident[EI_CLASS] != ELFCLASS64)
			return KM_RESULT_MALFORMED;
	}

	kfxx::dynarray_t<Elf64_Phdr> loaded_phdrs(kfxx::kernel_allocator());

	Elf64_Half phdr_num = ehdr.e_phnum;
	if (!loaded_phdrs.resize(phdr_num))
		return KM_RESULT_NO_MEM;

	for (Elf64_Half i = 0; i < phdr_num; ++i) {
		// Current program header.
		if (KM_FAILED(result = fs_read(file_fp, &loaded_phdrs.at(i), sizeof(Elf64_Phdr), ehdr.e_phoff + ehdr.e_phentsize * i, &bytes_read))) {
			return result;
		}
	}

	kfxx::dynarray_t<Elf64_Dyn> dyn_entries(kfxx::kernel_allocator());
	size_t max_size = 0;

	for (auto &i : loaded_phdrs) {
		switch (i.p_type) {
			case PT_LOAD: {
				if (i.p_filesz > i.p_memsz)
					return KM_RESULT_MALFORMED;
				break;
			}
			case PT_INTERP:
				kd_println(__func__, "Trying to load a kernel module with PT_INTERP segments");
				return KM_RESULT_INVALID_ARGS;
			case PT_DYNAMIC: {
				if (!dyn_entries.resize(kfxx::ceil_align_to(i.p_filesz, alignof(Elf64_Dyn)))) {
					return KM_RESULT_NO_MEM;
				}
				if (KM_FAILED(result = fs_read(file_fp, dyn_entries.data(), i.p_filesz, i.p_offset, &bytes_read))) {
					return result;
				}
				break;
			}
		}

		size_t seg_max_addr = i.p_vaddr + i.p_memsz;
		if (seg_max_addr >= max_size)
			max_size = seg_max_addr;
	}

	char *vaddr_base = (char *)mm_kvmalloc(mm_context, max_size, MM_PAGE_MAPPED, 0),
		 *vaddr_limit = vaddr_base + (max_size - 1);
	if (!vaddr_base)
		return KM_RESULT_NO_MEM;

	auto check_if_addr_is_inside_valid_region = [vaddr_base, vaddr_limit](const void *vaddr) -> bool {
		if (((const char *)vaddr >= vaddr_base) && ((const char *)vaddr <= vaddr_limit))
			return true;
		return false;
	};

	size_t mapped_phdr_idx = 0;
	kfxx::scope_guard unmap_guard([mm_context, vaddr_base, max_size]() noexcept {
		km_unwrap_result(mm_unmmap(mm_context, vaddr_base, max_size, 0));
	});
	while (mapped_phdr_idx < loaded_phdrs.size()) {
		auto &phdr = loaded_phdrs.at(mapped_phdr_idx);
		if (phdr.p_type == PT_LOAD) {
			char *split_point = vaddr_base + phdr.p_vaddr;

			size_t area_size;
			area_size = phdr.p_memsz;

			for (size_t i = 0; i < area_size; i += PAGESIZE) {
				void *cur_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
				kfxx::scope_guard free_paddr_guard([cur_paddr]() noexcept {
					mm_unpin_page(cur_paddr);
				});
				KM_RETURN_IF_FAILED(mm_mmap(mm_context, split_point + i, cur_paddr, page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, MMAP_NO_INC_RC));
				free_paddr_guard.release();
			}

			mm_pgaccess_t pgaccess = MM_PAGE_MAPPED;

			if (phdr.p_flags & PF_R)
				pgaccess |= MM_PAGE_READ;
			if (phdr.p_flags & PF_W)
				pgaccess |= MM_PAGE_WRITE;
			if (phdr.p_flags & PF_X)
				pgaccess |= MM_PAGE_EXEC;

			memset(split_point, 0, phdr.p_memsz);
			if (KM_FAILED(result = fs_read(file_fp, split_point, phdr.p_filesz, phdr.p_offset, &bytes_read))) {
				return result;
			}

			char &point = *split_point;
			point = point;

			km_unwrap_result(mm_set_page_access(mm_context, split_point, area_size, pgaccess));
		}
		++mapped_phdr_idx;
	}

	Elf64_Sym *symtab = nullptr;
	const char *strtab = nullptr;
	Elf64_Rela *rela = nullptr, *jmprel = nullptr;
	uint64_t rela_size = 0, jmprel_size = 0;

	for (size_t i = 0; i < dyn_entries.size(); ++i) {
		auto &entry = dyn_entries.at(i);

		if (entry.d_tag == DT_NULL)
			break;
		switch (entry.d_tag) {
			case DT_SYMTAB:
				symtab = (Elf64_Sym *)(vaddr_base + entry.d_un.d_ptr);
				break;
			case DT_STRTAB:
				strtab = (const char *)(vaddr_base + entry.d_un.d_ptr);
				break;
			case DT_RELA:
				rela = (Elf64_Rela *)(vaddr_base + entry.d_un.d_ptr);
				break;
			case DT_RELASZ:
				rela_size = entry.d_un.d_val;
				break;
			case DT_JMPREL:
				jmprel = (Elf64_Rela *)(vaddr_base + entry.d_un.d_ptr);
				break;
			case DT_PLTRELSZ:
				jmprel_size = entry.d_un.d_val;
				break;
		}
	}

	if (!symtab)
		return KM_RESULT_MALFORMED;
	if (!strtab)
		return KM_RESULT_MALFORMED;

	if (symtab && !check_if_addr_is_inside_valid_region(symtab))
		return KM_RESULT_MALFORMED;
	if (strtab && !check_if_addr_is_inside_valid_region(strtab))
		return KM_RESULT_MALFORMED;
	if (rela && !check_if_addr_is_inside_valid_region(rela))
		return KM_RESULT_MALFORMED;
	if (rela && !check_if_addr_is_inside_valid_region(((char *)rela) + rela_size))
		return KM_RESULT_MALFORMED;
	if (jmprel && !check_if_addr_is_inside_valid_region(((char *)jmprel) + jmprel_size))
		return KM_RESULT_MALFORMED;

	if (rela && rela_size) {
		size_t count = rela_size / sizeof(Elf64_Rela);
		for (size_t i = 0; i < count; i++) {
			Elf64_Addr *loc = (Elf64_Addr *)(vaddr_base + rela[i].r_offset);

			if (!check_if_addr_is_inside_valid_region(loc))
				return KM_RESULT_MALFORMED;

			KM_RETURN_IF_FAILED(mm_probe_kernel_pages(mm_context, loc, page_size, MM_PAGE_WRITE));

			Elf64_Xword type = ELF64_R_TYPE(rela[i].r_info);
			Elf64_Xword sym_idx = ELF64_R_SYM(rela[i].r_info);

			switch (type) {
				case R_X86_64_RELATIVE:
					// Write relative address
					*loc = (uint64_t)(vaddr_base + rela[i].r_addend);
					break;

				case R_X86_64_GLOB_DAT: {
					// Write GOT.
					const char *name = strtab + symtab[sym_idx].st_name;
					void *sym_addr = ps_get_kernel_symbol(name, strlen(name));
					if (!sym_addr) {
						kd_println(__func__, "Unresolved symbol: %s", name);
						return KM_RESULT_UNRESOLVED_SYMBOL;
					}
					*loc = (Elf64_Addr)sym_addr + rela[i].r_addend;
					break;
				}
				default:
					return KM_RESULT_MALFORMED;
			}
		}
	}

	if (jmprel && jmprel_size) {
		size_t count = jmprel_size / sizeof(Elf64_Rela);
		for (size_t i = 0; i < count; i++) {
			Elf64_Addr *loc = (Elf64_Addr *)(vaddr_base + jmprel[i].r_offset);

			if (!check_if_addr_is_inside_valid_region(loc))
				return KM_RESULT_MALFORMED;

			KM_RETURN_IF_FAILED(mm_probe_kernel_pages(mm_context, loc, page_size, MM_PAGE_WRITE));

			Elf64_Xword type = ELF64_R_TYPE(jmprel[i].r_info);
			Elf64_Xword sym_idx = ELF64_R_SYM(jmprel[i].r_info);

			if (type != R_X86_64_JUMP_SLOT)
				continue;

			{
				const char *name = strtab + symtab[sym_idx].st_name;
				void *func = ps_get_kernel_symbol(name, strlen(name));
				if (!func) {
					kd_println(__func__, "Unresolved symbol: %s", name);
					return KM_RESULT_UNRESOLVED_SYMBOL;
				}
				*loc = (Elf64_Addr)func + jmprel[i].r_addend;
			}
		}
	}

	kfxx::dynarray_t<Elf64_Shdr> loaded_shdrs(kfxx::kernel_allocator());

	Elf64_Half shdr_num = ehdr.e_shnum;
	if (!loaded_shdrs.resize(shdr_num))
		return KM_RESULT_NO_MEM;

	for (Elf64_Half i = 0; i < shdr_num; ++i) {
		if (KM_FAILED(result = fs_read(file_fp, &loaded_shdrs.at(i), sizeof(Elf64_Shdr), ehdr.e_shoff + ehdr.e_shentsize * i, &bytes_read))) {
			return result;
		}
	}

	size_t num_syms = 0;

	for (size_t i = 0; i < loaded_shdrs.size(); ++i) {
		auto &sh = loaded_shdrs.at(i);

		switch (sh.sh_type) {
			case SHT_DYNSYM: {
				if (num_syms)
					return KM_RESULT_MALFORMED;
				num_syms = sh.sh_size / sizeof(Elf64_Sym);
				break;
			}
			default:
				break;
		}
	}

	auto find_func = [vaddr_base](
						 Elf64_Sym *symtab, const char *strtab,
						 size_t num_syms, const char *name) -> std::pair<void *, uint64_t> {
		for (unsigned i = 0; i < num_syms; i++) {
			if (symtab[i].st_name != 0 &&
				strcmp(strtab + symtab[i].st_name, name) == 0) {
				if (ELF64_ST_TYPE(symtab[i].st_info) == STT_FUNC)
					return { (void *)(vaddr_base + symtab[i].st_value), (uint64_t)symtab[i].st_size };
			}
		}
		return { nullptr, 0 };
	};
	auto find_obj = [vaddr_base](
						Elf64_Sym *symtab, const char *strtab,
						size_t num_syms, const char *name) -> std::pair<void *, uint64_t> {
		for (unsigned i = 0; i < num_syms; i++) {
			if (symtab[i].st_name != 0 &&
				strcmp(strtab + symtab[i].st_name, name) == 0) {
				if (ELF64_ST_TYPE(symtab[i].st_info) == STT_OBJECT)
					return { (void *)(vaddr_base + symtab[i].st_value), (uint64_t)symtab[i].st_size };
			}
		}
		return { nullptr, 0 };
	};

	km_result_t (*module_init)();
	void (*module_deinit)();
	const char *module_name;
	uint64_t module_init_len = 0, module_deinit_len = 0, module_name_len = 0;
	{
		auto tmp = find_func(symtab, strtab, num_syms, "pbos_module_init");
		module_init = (km_result_t(*)())tmp.first;
		module_init_len = tmp.second;
	}
	{
		auto tmp = find_func(symtab, strtab, num_syms, "pbos_module_deinit");
		module_deinit = (void (*)())tmp.first;
		module_deinit_len = tmp.second;
	}
	{
		auto tmp = find_obj(symtab, strtab, num_syms, "PBOS_MODULE_NAME");
		module_name = (const char *)tmp.first;
		module_name_len = tmp.second;
	}

	if (!module_init) {
		kd_println(__func__, "pbos_module_init not found in the module, unloading...");
		return KM_RESULT_MALFORMED;
	}

	if (!module_deinit) {
		kd_println(__func__, "pbos_module_deinit not found in the module, unloading...");
		return KM_RESULT_MALFORMED;
	}

	if (!module_name) {
		kd_println(__func__, "PBOS_MODULE_NAME not found in the module, unloading...");
		return KM_RESULT_MALFORMED;
	}

	for (size_t i = 0; i < module_init_len; i += page_size) {
		if (!check_if_addr_is_inside_valid_region(((char *)module_init + i)))
			return KM_RESULT_MALFORMED;
	}

	for (size_t i = 0; i < module_deinit_len; i += page_size) {
		if (!check_if_addr_is_inside_valid_region(((char *)module_deinit + i)))
			return KM_RESULT_MALFORMED;
	}

	for (size_t i = 0; i < module_name_len; i += page_size) {
		if (!check_if_addr_is_inside_valid_region((module_name + i)))
			return KM_RESULT_MALFORMED;
	}

	if (!module_name_len)
		return KM_RESULT_MALFORMED;

	if (module_name[module_name_len - 1] == '\0') {
		if (!(module_name_len = strlen(module_name)))
			return KM_RESULT_MALFORMED;
	}

	KM_RETURN_IF_FAILED(ps_set_kmod_name(kmod, module_name, module_name_len));

	ps_set_kmod_init_fn(kmod, module_init);
	ps_set_kmod_deinit_fn(kmod, module_deinit);

	KM_RETURN_IF_FAILED(ps_add_section_to_kmod(kmod, vaddr_base, max_size, nullptr));

	{
		size_t name_len;
		kd_println(__func__, "Loaded kernel module '%s' at %p-%p", ps_get_kmod_name(kmod, &name_len), vaddr_base, vaddr_limit);
	}

	unmap_guard.release();

	return KM_RESULT_OK;
}

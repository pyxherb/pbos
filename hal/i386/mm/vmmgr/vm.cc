#include "vm.h"
#include <hal/i386/logger.h>
#include <pbos/km/proc.h>
#include <hal/i386/mm.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

arch_pte_t *const mm_kernel_ptt = (arch_pte_t *)KALLPGTAB_VBASE;

static uint16_t hn_pgaccess_to_pgmask(mm_pgaccess_t access) {
	uint16_t mask = 0;

	if (access & PAGE_MAPPED)
		mask |= PTE_P;
	// We cannot set if a page frame is read-only on x86  :(
	if (access & PAGE_WRITE)
		mask |= PTE_RW;
	if (access & PAGE_NOCACHE)
		mask |= PTE_CD;
	if (access & PAGE_USER)
		mask |= PTE_U;
	// Only works on processors that support DEP.
	if (!(access & PAGE_EXEC))
		mask |= PTE_XD;

	return mask;
}

static mm_pgaccess_t hn_pgmask_to_pgaccess(uint16_t mask) {
	mm_pgaccess_t access;

	// We cannot set if a page frame is read-only on x86  :(
	if (mask & PTE_P)
		access |= PAGE_MAPPED;
	if (mask & PTE_RW)
		access |= PAGE_WRITE;
	if (mask & PTE_CD)
		access |= PAGE_NOCACHE;
	if (mask & PTE_U)
		access |= PAGE_USER;
	if (!(mask & PTE_XD))
		access |= PAGE_EXEC;

	return access;
}

void *mm_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags) {
	return mm_vmalloc(ctxt, ((uint8_t *)KRESERVED_VTOP) + 1,
		(void *)((char *)KALLPGTAB_VBASE - 1), size, access, flags);
}

void mm_vmfree(mm_context_t *ctxt, void *addr, size_t size) {
	mm_unmmap(ctxt, addr, size, 0);
}

PBOS_NODISCARD km_result_t hn_mm_mmap_early(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access) {
	const void *vaddr_limit = ((const char *)vaddr) + (size - 1);

	for (uint16_t i = PDX(vaddr); i <= PDX(vaddr_limit); ++i) {
		arch_pde_t *pde = &context->pdt[i];
		if (!(pde->mask & PDE_P)) {
			kd_assert(hn_mm_init_stage >= HN_MM_INIT_STAGE_INITIAL_AREAS_INITED);
			void *target_ptr = ((char *)KALLPGTAB_VBASE) + PAGESIZE * i;
			// This manages 4MB size of memory.
			void *pgdir = kn_mm_alloc_pgdir_page(context, VADDR(i, 0, 0), 0);

			if (!pgdir)
				return KM_MAKEERROR(KM_RESULT_NO_MEM);
			else {
				{
					pgaddr_t mapped_pgdir = hn_tmpmap(PGROUNDDOWN(pgdir), 1, PTE_P | PTE_RW);
					arch_pte_t *pte = (arch_pte_t *)UNPGADDR(mapped_pgdir);

					memset(pte, 0, sizeof(arch_pte_t) * (PTX_MAX + 1));

					hn_tmpunmap(mapped_pgdir);
				}
				pde->mask = PDE_P | PDE_RW | PDE_U;

				hn_mad_t *mad = hn_get_mad(PGROUNDDOWN(pgdir));
				hn_set_pgblk_used(PGROUNDDOWN(pgdir), MAD_ALLOC_KERNEL);

				km_unwrap_result(hn_mm_mmap_early(context, target_ptr, pgdir, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE));
			}

			arch_invlpg(target_ptr);
		}
	}

	uint16_t mask = hn_pgaccess_to_pgmask(access);
	{
		size_t i = PGROUNDDOWN(vaddr), end = PGROUNDUP(((uintptr_t)vaddr) + size);
		uintptr_t pi = (uintptr_t)PGROUNDDOWN(paddr);
		ptrdiff_t pi_diff = paddr ? 1 : 0;
		while (i < end) {
			kd_assert(context->pdt[PDX(UNPGADDR(i))].mask & PDE_P);
			pgaddr_t t = hn_tmpmap(context->pdt[PDX(UNPGADDR(i))].address, 1, PTE_P | PTE_RW);
			arch_pte_t *const pte = &((arch_pte_t *)UNPGADDR(t))[PTX(UNPGADDR(i))];

			pte->address = pi;
			pte->mask = mask;

			arch_invlpg(UNPGADDR((uintptr_t)i));

			++i;

			pi += pi_diff;

			hn_tmpunmap(t);
		}
	}
	return KM_RESULT_OK;
}

[[nodiscard]] static km_result_t hn_do_mmap(
	mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	bool is_user_space = mm_is_user_space(vaddr);
	bool is_cur_pgtab = (ctxt == mm_get_cur_context());
	uint16_t mask = hn_pgaccess_to_pgmask(access);
	pgaddr_t i = PGROUNDDOWN(vaddr), end = PGROUNDUP(((uintptr_t)vaddr) + size);
	pgaddr_t pi = PGROUNDDOWN(paddr);
	pgaddr_t pi_diff = paddr ? 1 : 0;

	while (i < end) {
		arch_pte_t *pte;

		kfxx::scope_guard sg([&pte]() noexcept {
			mm_unmmap(mm_get_cur_context(), pte, PAGESIZE, 0);
		});

		if ((is_cur_pgtab || (!is_user_space))) {
			if ((UNPGADDR(i) >= (void *)KALLPGTAB_VBASE) &&
				(UNPGADDR(i) <= (void *)KALLPGTAB_VTOP) &&
				(!is_cur_pgtab))
				goto map;
			pte = &mm_kernel_ptt[i];
			sg.release();
		} else {
		map:
			pte = (arch_pte_t *)mm_kvmalloc(mm_get_cur_context(), PAGESIZE, PAGE_READ | PAGE_WRITE, 0);
			if (!pte) {
				sg.release();
				return KM_RESULT_NO_MEM;
			}
			km_unwrap_result(
				mm_mmap(mm_get_cur_context(), pte, UNPGADDR(ctxt->pdt[PDX(UNPGADDR(i))].address), PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0));
			pte += PTX(UNPGADDR(i));
		}

		if (pte->mask & PTE_P) {
			if (flags & MMAP_NOREMAP) {
				km_panic("Remapping virtual address %p with MMAP_NOREMAP", UNPGADDR(i));
			}

			// Free previous mapping.
			if (pte->address) {
				if (!(flags & MMAP_NORC)) {
					mm_pgfree(UNPGADDR(pte->address));
				}
			}

			if (!(mask & PTE_P))
				if (!(flags & MMAP_NOSETVPM)) {
					kn_mm_free_vpm(ctxt, UNPGADDR(i));
				}
		}

		pte->address = pi;
		pte->mask = mask;

		if (is_cur_pgtab || (!is_user_space))
			arch_invlpg(UNPGADDR(i));

		if (paddr) {
			if (!(flags & MMAP_NORC)) {
				mm_refpg(UNPGADDR(pi));
			}
		}

		i += 1;

		pi += pi_diff;
	}

	return KM_RESULT_OK;
}

km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	io::irq_disable_lock irq_lock;

	if (!ctxt)
		km_panic("Cannot call mm_mmap with null context");
	if (!size)
		km_panic("Cannot call mm_mmap with size == 0");

	kd_assert(hn_mm_init_stage >= HN_MM_INIT_STAGE_INITIAL_AREAS_INITED);

	bool is_cur_pgtab = (ctxt == mm_get_cur_context());
	bool is_user_space = mm_is_user_space(vaddr);
	if (is_user_space !=
		mm_is_user_space((void *)(((uintptr_t)vaddr) + (size - 1))))
		km_panic("Cannot map across user and kernel spaces");

	const void *vaddr_limit = ((const char *)vaddr) + (size - 1);

	kfxx::scope_guard unmap_guard([ctxt, vaddr, size, flags]() noexcept {
		mm_unmmap(ctxt, vaddr, size, flags);
	});

	while (true) {
		const char *prev_pgtab_vaddr = nullptr,
				   *pgtab_vaddr = (char *)vaddr, *pgtab_vaddr_limit = (char *)vaddr_limit;
		void *target_ptr;
		size_t iteration_times = 0;

	realloc_pgtab:
		for (uint16_t i = PDX(pgtab_vaddr); i <= PDX(pgtab_vaddr_limit); ++i) {
			target_ptr = ((char *)KALLPGTAB_VBASE) + PAGESIZE * i;
			arch_pde_t *pde = &ctxt->pdt[i];
			if (!(pde->mask & PDE_P)) {
				++iteration_times;

				if (prev_pgtab_vaddr != pgtab_vaddr) {
					prev_pgtab_vaddr = pgtab_vaddr;
					pgtab_vaddr = ((char *)KALLPGTAB_VBASE) + PAGESIZE * PDX(target_ptr);
					pgtab_vaddr_limit = ((char *)KALLPGTAB_VBASE) + PAGESIZE * PDX(target_ptr);

					goto realloc_pgtab;
				}
			}
		}

		if (iteration_times) {
			klog_printf("Allocating pgdir: %p\n", VADDR(PDX(prev_pgtab_vaddr), 0, 0));
			target_ptr = ((char *)KALLPGTAB_VBASE) + PAGESIZE * PDX(prev_pgtab_vaddr);
			// This manages 4MB size of memory.
			void *pgdir = kn_mm_alloc_pgdir_page(ctxt, VADDR(PDX(prev_pgtab_vaddr), 0, 0), 0);

			if (!pgdir)
				return KM_MAKEERROR(KM_RESULT_NO_MEM);
			else {
				{
					pgaddr_t mapped_pgdir = hn_tmpmap(PGROUNDDOWN(pgdir), 1, PTE_P | PTE_RW);
					arch_pte_t *pte = (arch_pte_t *)UNPGADDR(mapped_pgdir);

					memset(pte, 0, sizeof(arch_pte_t) * (PTX_MAX + 1));

					hn_tmpunmap(mapped_pgdir);
				}
				ctxt->pdt[PDX(prev_pgtab_vaddr)].mask = PDE_P | PDE_RW | PDE_U;

				hn_mad_t *mad = hn_get_mad(PGROUNDDOWN(pgdir));
				hn_set_pgblk_used(PGROUNDDOWN(pgdir), MAD_ALLOC_KERNEL);
			}

			// The pgtab_vaddr will converge to one address, it's the first page table need to be mapped.
			km_unwrap_result(hn_do_mmap(ctxt, (void *)target_ptr, UNPGADDR(ctxt->pdt[PDX(prev_pgtab_vaddr)].address), PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, MMAP_NOREMAP));
		} else
			break;
	}

	km_result_t result = hn_do_mmap(ctxt, vaddr, paddr, size, access, flags);

	if (KM_FAILED(result))
		return result;

	if (!(flags & MMAP_NOSETVPM)) {
		// Insert VPMs after free all previous VPMs.
		char *rounded_vaddr = (char *)PGFLOOR(vaddr), *rounded_paddr = (char *)PGFLOOR(paddr);
		size_t rounded_size = PGCEIL(size);
		for (size_t i = 0; i < rounded_size; i += PAGESIZE) {
			void *cur_ptr = rounded_vaddr + i;
			km_result_t result = kn_mm_insert_vpm(ctxt, cur_ptr);
			kd_assert(KM_SUCCEEDED(result));
		}
	}

	unmap_guard.release();

	return KM_RESULT_OK;
}

void mm_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	kd_assert(ctxt);

	io::irq_disable_lock irq_lock;

	bool is_cur_pgtab = (ctxt == mm_get_cur_context());

	{
		size_t i = PGROUNDDOWN(vaddr), end = PGROUNDUP(((uintptr_t)vaddr) + size);
		while (i < end) {
			arch_pte_t *const pte = &mm_kernel_ptt[i];
			if (pte->mask & PTE_P) {
				if (pte->address) {
					if (!(flags & MMAP_NORC)) {
						mm_pgfree(UNPGADDR(pte->address));
					}
				}
				if (!(flags & MMAP_NOSETVPM)) {
					kn_mm_free_vpm(ctxt, UNPGADDR(i));
				}
			}

			pte->mask &= ~PTE_P;

			if (is_cur_pgtab)
				arch_invlpg(UNPGADDR((uintptr_t)vaddr));
			++i;
		}
	}
}

void mm_set_page_access(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
	io::irq_disable_lock irq_lock;

	uint16_t mask = hn_pgaccess_to_pgmask(access);
	pgsize_t pg_num = PGROUNDUP(size);

	for (uint16_t di = PDX(vaddr); di < (PDX(vaddr) + PDX(pg_num + PTX_MAX));
		di++) {
		arch_pde_t *const pde = &context->pdt[di];
		if (!(pde->mask & PDE_P))
			km_panic("Modifying page access in non-existing PDE %p-%p for context %p",
				VADDR(di, 0, 0),
				VADDR(di, PTX_MAX, PGOFF_MAX),
				context);

		pgaddr_t tmapaddr = hn_tmpmap(pde->address, 1, PTE_P | PTE_RW);

		for (pgaddr_t ti = (VADDR(di, 0, 0) >= vaddr ? 0 : PTX(vaddr));
			(ti < (PTX_MAX + 1)) && ((PGADDR(di, ti) - PGROUNDDOWN(vaddr)) < pg_num);
			ti++) {
			// The page table was mapped temporarily.
			arch_pte_t *const pte = &(((arch_pte_t *)UNPGADDR(tmapaddr))[ti]);

			pte->mask = mask;

			arch_invlpg((void *)VADDR(di, ti, 0));
		}

		hn_tmpunmap(tmapaddr);
	}
}

void *mm_vmalloc(mm_context_t *ctxt,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags) {
	kd_assert(ctxt);
	kd_assert(size);
	io::irq_disable_lock irq_lock;
	pgaddr_t pgminaddr = PGROUNDDOWN(minaddr), pgmaxaddr = PGROUNDUP(maxaddr),
			 pgsize = PGROUNDUP(size);

	if ((pgminaddr > pgmaxaddr) || (pgmaxaddr - pgminaddr < pgsize)) {
		return NULL;
	}

	arch_pde_t *pdt = ctxt->pdt;

	void *p_found = NULL;	// Pointer to the free area.
	pgsize_t sz_found = 0;	// Size of free area found.

	for (uint16_t i = PDX(minaddr); i < PDX(maxaddr) + 1; ++i) {
		// Return if the size is enough.
		if (sz_found >= pgsize)
			goto succeeded;

		// Skip if the directory does not present
		if (!(pdt[i].mask & PDE_P)) {
			if (!p_found)
				p_found = VADDR(i, 0, 0);
			sz_found += PTX_MAX + 1;
			continue;
		}

		io::irq_disable_lock irq_lock;
		pgaddr_t tmapaddr = hn_tmpmap(
			pdt[i].address,
			PGROUNDUP(sizeof(arch_pte_t) * (PTX_MAX + 1)),
			PTE_P | PTE_RW);
		const arch_pte_t *pgtab = (arch_pte_t *)UNPGADDR(tmapaddr);

		// Scan for each PTEs.
		for (uint16_t j = (VADDR(i, 0, 0) >= minaddr ? 0 : PTX(minaddr));
			j < (PTX_MAX + 1) && (VADDR(i, j, 0) <= maxaddr); ++j) {
			if (pgtab[j].mask & PTE_P)
				// Clear the counter once we found any used page.
				p_found = NULL, sz_found = 0;
			else {
				if (!p_found)
					p_found = VADDR(i, j, 0);
				if (++sz_found >= pgsize) {
					hn_tmpunmap(tmapaddr);
					goto succeeded;
				}
			}
		}

		hn_tmpunmap(tmapaddr);
	}

	return NULL;
succeeded:
	mmap_flags_t mmap_flags = 0;

	if (flags & VMALLOC_NOSETVPM)
		mmap_flags |= VMALLOC_NOSETVPM;

	if (!(flags & VMALLOC_NORESERVE)) {
		if (KM_FAILED(mm_mmap(ctxt, p_found, NULL, size, PAGE_MAPPED | access, mmap_flags | MMAP_NOREMAP)))
			kd_assert(false);
	}
	return p_found;
}

void *hn_getmap(const arch_pde_t *pgdir, const void *vaddr, uint16_t *mask_out) {
	const arch_pde_t *pde = &pgdir[PDX(vaddr)];
	if (!(pde->mask & PDE_P))
		return NULL;

	pgaddr_t tmapaddr = hn_tmpmap(pde->address, 1, PTE_P);

	const arch_pte_t *pte = &((arch_pte_t *)UNPGADDR(tmapaddr))[PTX(vaddr)];

	if (mask_out)
		*mask_out = pte->mask;

	if (!(pte->mask & PTE_P)) {
		hn_tmpunmap(tmapaddr);
		return NULL;
	}

	char *paddr = (char *)UNPGADDR(pte->address);
	if (paddr)
		paddr += PGOFF(vaddr);

	hn_tmpunmap(tmapaddr);

	return paddr;
}

void *mm_getmap(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	io::irq_disable_lock irq_lock;

	kd_assert(ctxt);
	uint16_t mask;
	void *mapped_addr = hn_getmap(ctxt->pdt, vaddr, &mask);

	if (pgaccess_out) {
		*pgaccess_out = hn_pgmask_to_pgaccess(mask);
	}

	return mapped_addr;
}

typedef struct _tmpmap_info_t {
	pgaddr_t addr : 20;
	pgsize_t size : 20;
} tmpmap_info_t;

static tmpmap_info_t _tmpmap_slots[16];

pgaddr_t hn_tmpmap(pgaddr_t pgpaddr, pgsize_t pg_num, uint16_t mask) {
	kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpmap() requires interrupts disabled");
	kd_dbgcheck(
		pg_num <= (PGROUNDDOWN(KTMPMAP_SIZE)),
		"Number of pages (%d pages) to map is bigger than the size of KTMPMAP area (%d pages)",
		pg_num, PGROUNDDOWN(KTMPMAP_SIZE));

	kd_assert(ISVALIDPG(pgpaddr));

	pgaddr_t vaddr;

	// Because the virtual memory allocator also calls this function,
	// we have to find for free virtual memory by ourself.
	for (pgaddr_t i = PGROUNDDOWN(KTMPMAP_VBASE); i < PGROUNDUP(KTMPMAP_VTOP) - pg_num; ++i) {
		bool succeeded = true;

		for (uint8_t j = 0; j < PBOS_ARRAYSIZE(_tmpmap_slots); j++) {
			if (_tmpmap_slots[j].addr &&
				PBOS_ISOVERLAPPED(i, pg_num, _tmpmap_slots[j].addr, _tmpmap_slots[j].size)) {
				i = _tmpmap_slots[j].addr + _tmpmap_slots[j].size;
				succeeded = false;
				break;
			}
		}

		if (succeeded) {
			vaddr = i;
			goto alloc_succeeded;
		}
	}

	km_panic("Out of TMPMAP virtual memory");

alloc_succeeded:
	for (uint16_t i = 0; i < pg_num; ++i) {
		arch_pte_t *pte =
			&hn_kernel_pgt[i + PGROUNDDOWN(((char *)UNPGADDR(vaddr)) - KERNEL_VBASE)];
		pte->address = pgpaddr + i;
		pte->mask = mask;

		arch_invlpg(((char *)UNPGADDR(vaddr)) + (i << 12));
	}

	for (uint8_t i = 0; i < PBOS_ARRAYSIZE(_tmpmap_slots); ++i) {
		if (!_tmpmap_slots[i].addr) {
			_tmpmap_slots[i].addr = vaddr;
			_tmpmap_slots[i].size = pg_num;
			return vaddr;
		}
	}

	km_panic("TMPMAP slot is full");
}

void hn_tmpunmap(pgaddr_t addr) {
	kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpunmap() requires interrupts disabled");
	kd_dbgcheck(ISVALIDPG(addr), "Unmapping with an invalid TMPMAP address");

	tmpmap_info_t *i = NULL;
	for (uint8_t i = 0; i < PBOS_ARRAYSIZE(_tmpmap_slots); ++i) {
		if (_tmpmap_slots[i].addr == addr) {
			for (uint16_t j = 0; j < _tmpmap_slots[i].size; ++j) {
				arch_pte_t *cur_pte = &hn_kernel_pgt[PGROUNDDOWN(((char *)UNPGADDR(addr)) - CRITICAL_VBASE) + j];

				cur_pte->address = 0;
				cur_pte->mask = ~PTE_P;
				arch_invlpg(UNPGADDR(_tmpmap_slots[i].addr + j));
			}
			_tmpmap_slots[i].addr = 0;
			return;
		}
	}

	kd_dbgcheck(false, "Invalid TMPMAP address: %p", UNPGADDR(addr));
}

pgaddr_t hn_kvpgalloc(const arch_pde_t *pgdir) {
	return hn_vpgalloc(pgdir, PGROUNDUP(CRITICAL_VTOP + 1), PGADDR_MAX);
}

pgaddr_t hn_vpgalloc(const arch_pde_t *pgdir, pgaddr_t minaddr, pgaddr_t maxaddr) {
	kd_assert(ISVALIDPG(minaddr));
	kd_assert(ISVALIDPG(maxaddr));
	kd_assert(minaddr <= maxaddr);

	for (uint16_t i = PGDX(minaddr); i < PGDX(maxaddr) + 1; ++i) {
		// Skip if the directory does not present
		if (!(pgdir[i].mask & PDE_P))
			return PGADDR(i, 0);

		arch_pte_t *pgtab;
		pgaddr_t tmapaddr = NULLPG;

		{
			hn_vpm_t *vpm;
			if ((vpm = kn_mm_lookup_vpm(mm_get_cur_context(), UNPGADDR(pgdir[i].address), HN_VPM_LEVEL_MAX))) {
				if (vpm->map_addr) {
					pgtab = (arch_pte_t *)vpm->map_addr;
					goto already_mapped;
				}
			}

			tmapaddr = hn_tmpmap(pgdir[i].address, PGROUNDUP(sizeof(arch_pte_t) * (PTX_MAX + 1)),
				PTE_P | PTE_RW);
			pgtab = (arch_pte_t *)UNPGADDR(tmapaddr);
		}
	already_mapped:

		//
		// Scan for each PTEs.
		//
		// We check if the first PTE is in the range because
		// we have to let the initial page table index in the range,
		// and we have to check if we overrun the page table or the range
		// in the condition.
		//
		for (uint16_t j = (PGADDR(i, 0) >= minaddr ? 0 : PGTX(minaddr));
			j < (PTX_MAX + 1) && PGADDR(i, j) < (maxaddr + 1); ++j) {
			// The page is free if it does not present.
			if (!(pgtab[j].mask & PTE_P)) {
				hn_tmpunmap(tmapaddr);
				return PGADDR(i, j);
			}
		}

		if (tmapaddr != NULLPG) {
			hn_tmpunmap(tmapaddr);
		}
	}

	return NULLPG;
}

void *kn_lookup_pgdir(mm_context_t *ctxt, void *addr, int level) {
	switch (level) {
		case 0: {
			kd_assert(ctxt);
			kd_assert(PDX(addr) <= PDX_MAX);
			arch_pde_t *pde = &(ctxt->pdt[PDX(addr)]);

			if (!(pde->mask & PDE_P))
				return NULL;

			return UNPGADDR(pde->address);
		}
		default:
			km_panic("Invalid page level for page directory: %d", level);
	}
}

void *kn_mm_alloc_pgdir_page(mm_context_t *ctxt, void *ptr, int level) {
	switch (level) {
		case 0: {
			kd_assert(ctxt);
			kd_assert(PDX(ptr) <= PDX_MAX);
			arch_pde_t *pde = &(ctxt->pdt[PDX(ptr)]);

			kd_assert(!(pde->mask & PDE_P));

			if (!(pde->address = PGROUNDDOWN(
					  mm_pgalloc(MM_PMEM_AVAILABLE))))
				return NULL;

			return UNPGADDR(pde->address);
		}
		default:
			km_panic("Invalid page level for page directory: %d", level);
	}
}

void kn_mm_free_pgdir(mm_context_t *ctxt, void *ptr, int level) {
	switch (level) {
		case 0: {
			kd_assert(ctxt);
			kd_printf("Freeing page directory for context %p at PDX %hu: \n", ctxt, PDX(ptr));
			kd_assert(PDX(ptr) <= PDX_MAX);
			kd_assert(ctxt->pdt[PDX(ptr)].mask & PDE_P);
			mm_unmmap(ctxt, (void *)(KPDT_VBASE + PAGESIZE * PDX(ptr)), PAGESIZE, 0);
			mm_pgfree(UNPGADDR(ctxt->pdt[PDX(ptr)].address));
			ctxt->pdt[PDX(ptr)].mask &= ~PDE_P;
			break;
		}
		default:
			km_panic("Invalid page level for page directory: %d", level);
	}
}

PBOS_EXTERN_C_END

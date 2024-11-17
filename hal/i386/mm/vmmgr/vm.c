#include "vm.h"
#include <hal/i386/logger.h>
#include <hal/i386/mm.h>
#include <pbos/km/proc.h>

static uint16_t hn_pgaccess_to_pgmask(mm_pgaccess_t access) {
	uint16_t mask = PTE_P;

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

void *mm_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags) {
	return mm_vmalloc(ctxt, ((uint8_t *)KRESERVED_VTOP) + 1,
		(void *)KSPACE_VTOP, size, access, flags);
}

void mm_vmfree(mm_context_t *ctxt, void *addr, size_t size) {
	mm_unmmap(ctxt, addr, size, 0);
}

km_result_t hn_walkpgtab(arch_pde_t *pdt, void *vaddr, size_t size, hn_pgtab_walker walker, void *exargs) {
	km_result_t result;

	for (uint16_t di = PDX(vaddr); di <= (PDX(vaddr) + PDX(size));
		 di++) {
		arch_pde_t *const pde = &pdt[di];
		assert(pde->mask & PDE_P);

		pgaddr_t tmapaddr = hn_tmpmap(pde->address, 1, PTE_P | PTE_RW);

		for (uint16_t ti = VADDR(di, 0, 0) >= vaddr ? 0 : PTX(vaddr);
			 (ti <= PTX_MAX);
			 ++ti) {
			if ((((char *)VADDR(di, ti, 0)) - ((char *)vaddr)) >= size) {
				break;
			}

			// The page table will be mapped temporarily.
			arch_pte_t *const pte = &(((arch_pte_t *)UNPGADDR(tmapaddr))[ti]);

			if (KM_FAILED(result = walker(pde, pte, di, ti, exargs)))
				return result;
		}

		hn_tmpunmap(tmapaddr);
	}

	return KM_RESULT_OK;
}

typedef struct _hn_mmap_walker_args {
	char *paddr;
	uint16_t mask;
	bool is_curpgtab;
	mmap_flags_t flags;
	bool rawmap;
} hn_mmap_walker_args;

km_result_t hn_mmap_walker(arch_pde_t *pde, arch_pte_t *pte, uint16_t pdx, uint16_t ptx, void *exargs) {
	hn_mmap_walker_args *args = (hn_mmap_walker_args *)exargs;

	if (!(args->flags & MMAP_NORC)) {
		if (pte->mask & PTE_P) {
			if (pte->address) {
				mm_pgfree(UNPGADDR(pte->address));
			}
		}
	}

	pte->address = PGROUNDDOWN(args->paddr);
	pte->mask = args->mask;

	if (args->paddr)
		args->paddr += PAGESIZE;

	if (args->is_curpgtab)
		arch_invlpg((void *)VADDR(pdx, ptx, 0));

	return KM_RESULT_OK;
}

km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	const void *vaddr_limit = ((const char *)vaddr) + size;

	for (uint16_t i = PDX(vaddr); i <= PDX(vaddr_limit); ++i) {
		if (!(ctxt->pdt[i].mask & PDE_P)) {
			pgaddr_t pgdir = hn_mmctxt_pgtaballoc(ctxt, i);
			if (!pgdir) {
				mm_unmmap(ctxt, vaddr, size, flags);
				return KM_RESULT_NO_MEM;
			} else {
				pgaddr_t mapped_pgdir = hn_tmpmap(pgdir, 1, PTE_P | PTE_RW);
				arch_pte_t *pgdir_entries = (arch_pte_t *)UNPGADDR(mapped_pgdir);

				memset(pgdir_entries, 0, sizeof(arch_pte_t) * (PTX_MAX + 1));

				hn_tmpunmap(mapped_pgdir);
			}
			ctxt->pdt[i].mask = PDE_P | PDE_RW | PDE_U;
		}
	}

	hn_mmap_walker_args args = {
		.paddr = (char *)paddr,
		.mask = hn_pgaccess_to_pgmask(access),
		.is_curpgtab = ctxt == mm_current_contexts[ps_get_current_euid()],
		.flags = flags
	};
	return hn_walkpgtab(ctxt->pdt, vaddr, size, hn_mmap_walker, &args);
}

km_result_t mm_mrawmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
}

typedef struct _hn_unmmap_walker_args {
	bool is_curpgtab;
	mmap_flags_t flags;
	bool rawmap;
} hn_unmmap_walker_args;

km_result_t hn_unmmap_walker(arch_pde_t *pde, arch_pte_t *pte, uint16_t pdx, uint16_t ptx, void *exargs) {
	hn_unmmap_walker_args *args = (hn_unmmap_walker_args *)exargs;

	if (!(args->flags & MMAP_NORC)) {
		if (pte->mask & PTE_P) {
			if (pte->address) {
				mm_pgfree(UNPGADDR(pte->address));
			}
		}
	}

	pte->mask &= ~PTE_P;

	if (args->is_curpgtab)
		arch_invlpg((void *)VADDR(pdx, ptx, 0));

	return KM_RESULT_OK;
}

void mm_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	const void *vaddr_limit = ((const char *)vaddr) + size;
	hn_unmmap_walker_args args = {
		.is_curpgtab = ctxt == mm_current_contexts[ps_get_current_euid()],
		.flags = flags
	};
	km_result_t result = hn_walkpgtab(ctxt->pdt, vaddr, size, hn_mmap_walker, &args);
	assert(KM_SUCCEEDED(result));

	for (uint16_t i = PDX(vaddr); i < PDX(vaddr_limit) + 1; ++i) {
		if (ctxt->pdt[i].mask & PDE_P) {
			if ((VADDR(i, 0, 0) >= vaddr) &&
				(VADDR(i, PTX_MAX, PGOFF_MAX) <= vaddr_limit)) {
				hn_mmctxt_pgtabfree(ctxt, i);
			} else {
				pgaddr_t pgtab_tmpmap_addr = hn_tmpmap(ctxt->pdt[i].address, 1, PTE_P);
				arch_pte_t *pgtab = UNPGADDR(pgtab_tmpmap_addr);

				for (uint16_t j = 0; j < PTX_MAX; ++j) {
					if (pgtab[j].mask & PTE_P) {
						goto keep_pgtab;
					}
				}

				hn_mmctxt_pgtabfree(ctxt, i);

			keep_pgtab:;
			}
		}
	}
}

void mm_mrawunmap(mm_context_t *context, void *vaddr, size_t size, mmap_flags_t flags) {
}

void mm_chpgmod(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
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
	assert(size);
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
	if (!(flags & VMALLOC_NORESERVE))
		mm_mmap(ctxt, p_found, NULL, size, access, 0);
	return p_found;
}

void *hn_getmap(const arch_pde_t *pgdir, const void *vaddr) {
	const arch_pde_t *pde = &pgdir[PDX(vaddr)];
	if (!(pde->mask & PDE_P))
		return NULL;

	pgaddr_t tmapaddr = hn_tmpmap(pde->address, 1, PTE_P);

	const arch_pte_t *pte = &((arch_pte_t *)UNPGADDR(tmapaddr))[PTX(vaddr)];
	if (!(pte->mask & PTE_P)) {
		hn_tmpunmap(tmapaddr);
		return NULL;
	}

	void *paddr = UNPGADDR(pte->address);
	if (paddr)
		paddr += PGOFF(vaddr);

	hn_tmpunmap(tmapaddr);

	return paddr;
}

void *mm_getmap(mm_context_t *ctxt, const void *vaddr) {
	return hn_getmap(ctxt->pdt, vaddr);
}

typedef struct _tmpmap_info_t {
	pgaddr_t addr : 20;
	pgsize_t size : 20;
} tmpmap_info_t;

static tmpmap_info_t _tmpmap_slots[16];

pgaddr_t hn_tmpmap(pgaddr_t pgpaddr, pgsize_t pg_num, uint16_t mask) {
	kd_dbgcheck(
		pg_num <= (PGROUNDDOWN(KTMPMAP_SIZE)),
		"Number of pages (%d pages) to map is bigger than the size of KTMPMAP area (%d pages)",
		pg_num, PGROUNDDOWN(KTMPMAP_SIZE));

	assert(ISVALIDPG(pgpaddr));

	pgaddr_t vaddr;

	// Because the virtual memory allocator also calls this function,
	// we have to find for free virtual memory by ourself.
	for (pgaddr_t i = PGROUNDDOWN(KTMPMAP_VBASE); i < PGROUNDUP(KTMPMAP_VTOP) - pg_num; ++i) {
		bool succeeded = true;

		for (uint8_t j = 0; j < PB_ARRAYSIZE(_tmpmap_slots); j++) {
			if (_tmpmap_slots[j].addr &&
				PB_ISOVERLAPPED(i, pg_num, _tmpmap_slots[j].addr, _tmpmap_slots[j].size)) {
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
			&hn_kernel_pgt[i + PGROUNDDOWN(UNPGADDR(vaddr) - KERNEL_VBASE)];
		pte->address = pgpaddr + i;
		pte->mask = mask;
	}

	for (pgsize_t i = 0; i < pg_num; ++i)
		arch_invlpg(UNPGADDR(vaddr) + (i << 12));

	for (uint8_t i = 0; i < PB_ARRAYSIZE(_tmpmap_slots); ++i) {
		if (!_tmpmap_slots[i].addr) {
			_tmpmap_slots[i].addr = vaddr;
			_tmpmap_slots[i].size = pg_num;
			return vaddr;
		}
	}

	km_panic("TMPMAP slot is full");
}

void hn_tmpunmap(pgaddr_t addr) {
	kd_dbgcheck(ISVALIDPG(addr), "Unmapping with an invalid TMPMAP address");

	tmpmap_info_t *i = NULL;
	for (uint8_t i = 0; i < PB_ARRAYSIZE(_tmpmap_slots); ++i) {
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
	assert(ISVALIDPG(minaddr));
	assert(ISVALIDPG(maxaddr));
	assert(minaddr <= maxaddr);

	for (uint16_t i = PGDX(minaddr); i < PGDX(maxaddr) + 1; ++i) {
		// Skip if the directory does not present
		if (!(pgdir[i].mask & PDE_P))
			return PGADDR(i, 0);

		pgaddr_t tmapaddr = hn_tmpmap(pgdir[i].address, PGROUNDUP(sizeof(arch_pte_t) * (PTX_MAX + 1)),
			PTE_P | PTE_RW);
		const arch_pte_t *pgtab = (arch_pte_t *)UNPGADDR(tmapaddr);

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
		hn_tmpunmap(tmapaddr);
	}

	return NULLPG;
}

pgaddr_t hn_mmctxt_pgtaballoc(mm_context_t *ctxt, uint16_t pdx) {
	kdprintf("Allocating page table for context %p at PDX %hu: \n", ctxt, pdx);
	assert(pdx <= PDX_MAX);
	arch_pde_t *pde = &(ctxt->pdt[pdx]);

	assert(!(pde->mask & PDE_P));

	if (!(pde->address = PGROUNDDOWN(
			  mm_pgalloc(MM_PMEM_AVAILABLE))))
		return (pgaddr_t)NULL;

	return ctxt->pdt[pdx].address;
}

void hn_mmctxt_pgtabfree(mm_context_t *ctxt, uint16_t pdx) {
	kdprintf("Freeing page table for context %p at PDX %hu: \n", ctxt, pdx);
	assert(pdx <= PDX_MAX);
	assert(ctxt->pdt[pdx].mask & PDE_P);
	mm_pgfree(UNPGADDR(ctxt->pdt[pdx].address));
	ctxt->pdt[pdx].mask = ~PDE_P;
}

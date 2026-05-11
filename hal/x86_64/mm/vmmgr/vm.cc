#include "vm.hh"
#include <pbos/km/logger.h>
#include <pbos/ps/proc.h>
#include <string.h>
#include <hal/x86_64/mm/pgalloc/pgalloc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

static uint16_t hn_pgaccess_to_pgmask(mm_pgaccess_t access) {
	uint16_t mask = 0;

	if (access & MM_PAGE_MAPPED)
		mask |= PTE_P;
	// We cannot set if a page frame is read-only on x86  :(
	if (access & MM_PAGE_WRITE)
		mask |= PTE_RW;
	if (access & MM_PAGE_NOCACHE)
		mask |= PTE_CD;
	if (access & MM_PAGE_USER)
		mask |= PTE_U;
	// if (!(access & MM_PAGE_EXEC))
	// mask |= PTE_XD;

	return mask;
}

void *kh_get_direct_mmap(void *paddr) {
	hn_pmad_t *pmad = hn_pmad_get(paddr);

	if ((!pmad) || (!pmad->direct_map_base))
		return nullptr;

	ptrdiff_t off = (((char *)paddr) - (char *)pmad->base);
	char *p = (char *)pmad->direct_map_base + off;

	if (p > (char *)DIRECTPHYMEM_VTOP)
		return nullptr;
	return p;
}

void *kh_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags) {
	return kh_vmalloc(ctxt, (void *)(DIRECTPHYMEM_VTOP + 1),
		(void *)KERNEL_VBASE, size, access, flags);
}

void mm_vmfree(mm_context_t *ctxt, void *addr, size_t size) {
	kh_unmmap(ctxt, addr, size, 0);
}

PBOS_NODISCARD uint8_t hn_mm_mmap_early(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	mm_pgaccess_t access,
	void *pdpte_paddr,
	void *pde_paddr,
	void *pte_paddr) {
	uint8_t result = 0;
	const uint8_t pgaccess = hn_pgaccess_to_pgmask(access);

	const size_t size = PAGESIZE;

	void *const addr_limit = (void *)PGCEIL((char *)vaddr + size);
	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);

	arch_pml4te_t *pml4t = (arch_pml4te_t *)context->page_table;

	size_t sz_mapped = 0;
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		// Return if the size is enough.
		if (sz_mapped >= size)
			break;

		arch_pml4te_t *pml4te = &pml4t[pml4x];
		bool init_pdpt = false;

		if (!(pml4te->mask & PML4E_P)) {
			result |= 0b100;
			pml4te->address = PGROUNDDOWN(pdpte_paddr);
			pml4te->mask = PML4E_P | PML4E_RW;
			init_pdpt = true;
		}

		arch_pdpte_t *pdpt = (arch_pdpte_t *)
			hn_tmpmap_early(
				UNPGADDR(pml4te->address),
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);

		kfxx::deferred release_pdpt_guard([&pdpt]() noexcept {
			hn_tmpunmap_early((void *)pdpt, PAGESIZE);
		});

		if (init_pdpt)
			memset(pdpt, 0, PAGESIZE);

		for (uint16_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			if (sz_mapped >= size)
				break;

			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));
			bool init_pdt = false;

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			if (!(pdpt->mask & PDPTE_P)) {
				result |= 0b010;
				pdpt[pdptx].address = PGROUNDDOWN(pde_paddr);
				pdpt[pdptx].mask = PDPTE_P | PDPTE_RW;
				init_pdt = true;
			}

			arch_pde_t *pdt = (arch_pde_t *)
				hn_tmpmap_early(
					UNPGADDR(pdpt[pdptx].address),
					sizeof(arch_pde_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);

			kfxx::deferred release_pde_guard([&pdt]() noexcept {
				hn_tmpunmap_early((void *)pdt, PAGESIZE);
			});

			if (init_pdt)
				memset(pdt, 0, PAGESIZE);

			for (uint16_t pdx =
					 (pml4x == PML4X(vaddr) &&
								 pdptx == PDPTX(vaddr)
							 ? PDX(vaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				if (sz_mapped >= size)
					break;

				char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));
				bool init_ptt = false;

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				if (!(pdt[pdx].mask & PDE_P)) {
					result |= 0b001;
					pdt[pdx].address = PGROUNDDOWN(pte_paddr);
					pdt[pdx].mask = PDE_P | PDE_RW;
					init_ptt = true;
				}

				arch_pte_t *ptt = (arch_pte_t *)
					hn_tmpmap_early(
						UNPGADDR(pdt[pdx].address),
						sizeof(arch_pte_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);

				if (init_ptt)
					memset(ptt, 0, PAGESIZE);

				kfxx::deferred release_pte_guard([&ptt]() noexcept {
					hn_tmpunmap_early((void *)ptt, PAGESIZE);
				});

				for (uint16_t ptx =
						 (pml4x == PML4X(vaddr) &&
									 pdptx == PDPTX(vaddr) &&
									 pdx == PDX(vaddr)
								 ? PTX(vaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					if (sz_mapped >= size)
						break;

					char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					ptt[ptx].address = PGROUNDDOWN((char *)paddr + sz_mapped);
					ptt[ptx].mask = pgaccess;
					ptt[ptx].xd = !(access & MM_PAGE_EXEC);

					arch_invlpg((char *)vaddr + sz_mapped);

					sz_mapped += PAGESIZE;
				}
			}
		}
	}

	return result;
}

km_result_t kh_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	// io::irq_disable_lock irq_lock;

#ifndef NDEBUG
	if (!ctxt)
		km_panic("Cannot call mmap with null context");
	if (!size)
		km_panic("Cannot call mmap with size == 0");
#endif

	void *const addr_limit = (char *)vaddr + size;
	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;
	bool is_cur_pgtab = ctxt == mm_get_cur_context();

#ifndef NDEBUG
	bool is_user_space = mm_is_user_space(vaddr);

	if (is_user_space !=
		mm_is_user_space((void *)(((uintptr_t)vaddr) + (size - 1))))
		km_panic("Cannot map across user and kernel spaces");
#endif

	char *pi = (char *)paddr;
	size_t pi_diff = paddr ? PAGESIZE : 0;
	uint8_t mask = hn_pgaccess_to_pgmask(access);

	kfxx::scope_guard unmmap_guard([ctxt, vaddr, size]() noexcept {
		kh_unmmap(ctxt, vaddr, size, 0);
	});

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &pml4t[pml4x];

		bool is_pml4te_allocated = false;
		if (!(pml4te->mask & PML4E_P)) {
			if (flags & MMAP_NO_PGTAB_ALLOC)
				km_panic("Missing PML4 table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
			// Allocate page table.
			if (!(pml4te->address = PGROUNDDOWN(mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)))) {
				return KM_RESULT_NO_MEM;
			}
			pml4te->mask = PML4E_P | PML4E_RW | PML4E_U;
			pml4te->xd = false;
			is_pml4te_allocated = true;
		}

		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(pml4te->address);
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hn_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hn_tmpmap_post(
					pdpt_paddr,
					sizeof(arch_pdpte_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);
		} else
			release_tmpmap_pdpt_guard.release();

		if (is_pml4te_allocated)
			memset(pdpt, 0, PAGESIZE);

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			bool is_pdpte_allocated = false;
			if (!(pdpt[pdptx].mask & PDPTE_P)) {
				if (flags & MMAP_NO_PGTAB_ALLOC)
					km_panic("Missing PDP table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
				// Allocate page table.
				if (!(pdpt[pdptx].address = PGROUNDDOWN(mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)))) {
					return KM_RESULT_NO_MEM;
				}
				pdpt[pdptx].mask = PDPTE_P | PDPTE_RW | PDPTE_U;
				pdpt[pdptx].xd = false;
				is_pdpte_allocated = true;
			}

			arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(pdpt[pdptx].address);
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hn_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hn_tmpmap_post(
						pdt_paddr,
						sizeof(arch_pde_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);
			} else
				release_tmpmap_pdt_guard.release();

			if (is_pdpte_allocated)
				memset(pdt, 0, PAGESIZE);

			// Walk each PDE.
			for (uint16_t pdx =
					 (pml4x == PML4X(vaddr) &&
								 pdptx == PDPTX(vaddr)
							 ? PDX(vaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				bool is_pde_allocated = false;
				if (!(pdt[pdx].mask & PDE_P)) {
					if (flags & MMAP_NO_PGTAB_ALLOC)
						km_panic("Missing page directory table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
					// Allocate page table.
					if (!(pdt[pdx].address = PGROUNDDOWN(mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)))) {
						return KM_RESULT_NO_MEM;
					}
					pdt[pdx].mask = PDE_P | PDE_RW | PDE_U;
					pdt[pdx].xd = false;
					is_pde_allocated = true;
				}

				arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(pdt[pdx].address);
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hn_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hn_tmpmap_post(
							ptt_paddr,
							sizeof(arch_pte_t) * (PTX_MAX + 1),
							PTE_P | PTE_RW);
				} else
					release_tmpmap_ptt_guard.release();

				if (is_pde_allocated)
					memset(ptt, 0, PAGESIZE);

				// Walk each PTE.
				for (uint16_t ptx =
						 (pml4x == PML4X(vaddr) &&
									 pdptx == PDPTX(vaddr) &&
									 pdx == PDX(vaddr)
								 ? PTX(vaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					arch_pte_t *pte = &ptt[ptx];

					if (pte->mask & PTE_P) {
						if (flags & MMAP_NO_REMAP)
							km_panic("Remapping address %p with MMAP_NO_REMAP", ptt_vaddr);

						if (!(flags & MMAP_NO_RC))
							mm_pgfree(UNPGADDR(pte->address));
					} else {
					}

					pte->address = PGROUNDDOWN(pi);
					pte->mask = mask;
					pte->xd = !(access & MM_PAGE_EXEC);

					pi += pi_diff;

					if (is_cur_pgtab)
						arch_invlpg(ptt_vaddr);
				}
			}
		}
	}

	unmmap_guard.release();

	return KM_RESULT_OK;
}

void kh_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	kd_assert(ctxt);

	// io::irq_disable_lock irq_lock;
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;
	void *const addr_limit = (char *)vaddr + size;
	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	bool is_cur_pgtab = ctxt == mm_get_cur_context();

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &pml4t[pml4x];

		if (!(pml4te->mask & PML4E_P))
			continue;

		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(pml4te->address);
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hn_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hn_tmpmap_post(
					UNPGADDR(pml4te->address),
					sizeof(arch_pdpte_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);
		} else
			release_tmpmap_pdpt_guard.release();

		const uint16_t initial_pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
		uint16_t pdptx = initial_pdptx;

		// Walk each PDPTE.
		for (;
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			if (!(pdpt[pdptx].mask & PDPTE_P))
				continue;

			arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(pml4te->address);
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hn_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hn_tmpmap_post(
						UNPGADDR(pdpt[pdptx].address),
						sizeof(arch_pde_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);
			} else
				release_tmpmap_pdt_guard.release();

			const uint16_t initial_pdx =
				(pml4x == PML4X(vaddr) &&
							pdptx == PDPTX(vaddr)
						? PDX(vaddr)
						: 0);
			uint16_t pdx = initial_pdx;
			// Walk each PDE.
			for (;
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				if (!(pdt[pdx].mask & PDE_P))
					continue;

				arch_pte_t *ptt;
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hn_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(pdt_paddr))) {
					ptt = (arch_pte_t *)
						hn_tmpmap_post(
							UNPGADDR(pdt[pdx].address),
							sizeof(arch_pte_t) * (PTX_MAX + 1),
							PTE_P | PTE_RW);
				} else
					release_tmpmap_ptt_guard.release();

				const uint16_t initial_ptx =
					(pml4x == PML4X(vaddr) &&
								pdptx == PDPTX(vaddr) &&
								pdx == PDX(vaddr)
							? PTX(vaddr)
							: 0);
				uint16_t ptx = initial_ptx;

				// Walk each PTE.
				for (;
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					arch_pte_t *pte = &ptt[ptx];

					if (pte->mask & PTE_P) {
						if (flags & MMAP_NO_REMAP)
							km_panic("Remapping address %p with MMAP_NO_REMAP", ptt_vaddr);

						if (!(flags & MMAP_NO_RC))
							mm_pgfree(UNPGADDR(pte->address));
					}

					pte->address = 0;
					pte->mask = 0;

					if (is_cur_pgtab)
						arch_invlpg(ptt_vaddr);
				}

				// Check if the page table is unneeded.
				for (uint16_t i = 0; i < initial_ptx; ++i) {
					if (ptt[i].mask & PTE_P)
						goto skip_release_pt;
				}

				for (uint16_t i = ptx; i < PTX_MAX; ++i) {
					if (ptt[i].mask & PTE_P)
						goto skip_release_pt;
				}

				pdt[pdx].mask = 0;
				mm_pgfree(UNPGADDR(pdt[pdx].address));

			skip_release_pt:;
			}

			// Check if the page directory is unneeded.
			for (uint16_t i = 0; i < initial_pdx; ++i) {
				if (pdt[i].mask & PDE_P)
					goto skip_release_pd;
			}

			for (uint16_t i = pdx; i < PTX_MAX; ++i) {
				if (pdt[i].mask & PDE_P)
					goto skip_release_pd;
			}

			pdpt[pdptx].mask = 0;
			mm_pgfree(UNPGADDR(pdpt[pdx].address));

		skip_release_pd:;
		}

		// Check if the PDP is unneeded.
		for (uint16_t i = 0; i < initial_pdptx; ++i) {
			if (pdpt[i].mask & PDPTE_P)
				goto skip_release_pdp;
		}

		for (uint16_t i = pdptx; i < PTX_MAX; ++i) {
			if (pdpt[i].mask & PDPTE_P)
				goto skip_release_pdp;
		}

		pml4t[pdptx].mask = 0;
		mm_pgfree(UNPGADDR(pml4t[pdptx].address));

	skip_release_pdp:;
	}
}

void kh_set_page_access(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
	// io::irq_disable_lock irq_lock;

	uint16_t mask = hn_pgaccess_to_pgmask(access);
	// TODO: Implement it.
}

void *kh_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags) {
	// io::irq_disable_lock irq_lock;
	void *const addr_limit = (void *)(PGCEIL((char *)maxaddr) - PGCEIL(size) - 1);
	uintptr_t addr_prefix = ADDR_PREFIX(minaddr);
	kd_dbgcheck(
		addr_prefix == ADDR_PREFIX(maxaddr),
		"The address prefix of the minimum address and the maximum address does not match");
	arch_pml4te_t *pml4t = (arch_pml4te_t *)context->page_table;

	void *p_found = NULL;  // Pointer to the free area.
	size_t sz_found = 0;   // Size of free area found.

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(minaddr); pml4x < PML4X(maxaddr) + 1; ++pml4x) {
		// Return if the size is enough.
		if (sz_found >= size)
			break;

		arch_pml4te_t *pml4te = &pml4t[pml4x];

		// Skip if does not present
		if (!(pml4te->mask & PML4E_P)) {
			if (!p_found) {
				if ((p_found = (void *)(addr_prefix | (uintptr_t)UVADDR(pml4x, 0, 0, 0, 0))) < minaddr)
					p_found = (void *)minaddr;
			}
			sz_found += (size_t)UVADDR(1, 0, 0, 0, 0);
			continue;
		}

		// io::irq_disable_lock irq_lock;
		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(pml4te->address);
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hn_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hn_tmpmap_post(
					pdpt_paddr,
					sizeof(arch_pdpte_t) * (PTX_MAX + 1),
					PTE_P);
		} else
			release_tmpmap_pdpt_guard.release();

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(minaddr) ? PDPTX(minaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			if (sz_found >= size)
				break;

			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			if (!(pdpt[pdptx].mask & PDPTE_P)) {
				if (!p_found) {
					if ((p_found = pdpt_vaddr) < minaddr)
						p_found = (void *)minaddr;
				}
				sz_found += (size_t)UVADDR(0, 1, 0, 0, 0);
				continue;
			}

			const arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(pdpt[pdptx].address);
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hn_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hn_tmpmap_post(
						pdt_paddr,
						sizeof(arch_pde_t) * (PTX_MAX + 1),
						PTE_P);
			} else
				release_tmpmap_pdt_guard.release();

			// Walk each PDE.
			for (uint16_t pdx =
					 (pml4x == PML4X(minaddr) &&
								 pdptx == PDPTX(minaddr)
							 ? PDX(minaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				if (sz_found >= size)
					break;

				char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				if (!(pdt[pdx].mask & PDE_P)) {
					if (!p_found) {
						if ((p_found = pdt_vaddr) < minaddr)
							p_found = (void *)minaddr;
					}
					sz_found += (size_t)UVADDR(0, 0, 1, 0, 0);
					continue;
				}

				const arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(pdt[pdx].address);
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hn_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hn_tmpmap_post(
							ptt_paddr,
							sizeof(arch_pte_t) * (PTX_MAX + 1),
							PTE_P);
				} else
					release_tmpmap_ptt_guard.release();

				// Walk each PTE.
				for (uint16_t ptx =
						 (pml4x == PML4X(minaddr) &&
									 pdptx == PDPTX(minaddr) &&
									 pdx == PDX(minaddr)
								 ? PTX(minaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					if (sz_found >= size)
						break;

					char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					if (!(ptt[ptx].mask & PTE_P)) {
						if (!p_found)
							p_found = ptt_vaddr;
						sz_found += (size_t)UVADDR(0, 0, 0, 1, 0);
						continue;
					}
				}
			}
		}
	}

	if (sz_found >= size) {
		km_unwrap_result(
			kh_mmap(context, p_found, nullptr, size, access, flags | MMAP_NO_REMAP));
		return p_found;
	}

	return nullptr;
}

PBOS_NODISCARD void *mm_kvmalloc_early(mm_context_t *context, size_t size, mm_pgaccess_t access) {
	return mm_vmalloc_early(context, (void *)(DIRECTPHYMEM_VTOP + 1),
		(void *)KERNEL_VBASE, size, access);
}

PBOS_NODISCARD void *mm_vmalloc_early(
	mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access) {
	void *const addr_limit = (void *)(PGCEIL((char *)maxaddr) - PGCEIL(size) - 1);
	uintptr_t addr_prefix = ADDR_PREFIX(minaddr);
	kd_dbgcheck(
		addr_prefix == ADDR_PREFIX(maxaddr),
		"The address prefix of the minimum address and the maximum address does not match");
	arch_pml4te_t *pml4t = (arch_pml4te_t *)context->page_table;

	void *p_found = NULL;  // Pointer to the free area.
	size_t sz_found = 0;   // Size of free area found.

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(minaddr); pml4x < PML4X(maxaddr) + 1; ++pml4x) {
		// Return if the size is enough.
		if (sz_found >= size)
			break;

		arch_pml4te_t *pml4te = &pml4t[pml4x];

		// Skip if does not present
		if (!(pml4te->mask & PML4E_P)) {
			if (!p_found)
				p_found = (void *)(addr_prefix | (uintptr_t)UVADDR(pml4x, 0, 0, 0, 0));
			sz_found += (size_t)UVADDR(1, 0, 0, 0, 0);
			continue;
		}

		// io::irq_disable_lock irq_lock;
		const arch_pdpte_t *pdpt = (arch_pdpte_t *)
			hn_tmpmap_early(
				UNPGADDR(pml4te->address),
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);

		kfxx::deferred release_pdpt_guard([pdpt]() noexcept {
			hn_tmpunmap_early((void *)pdpt, PAGESIZE);
		});

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(minaddr) ? PDPTX(minaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			if (sz_found >= size)
				break;

			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			if (!(pdpt[pdptx].mask & PDPTE_P)) {
				if (!p_found)
					p_found = pdpt_vaddr;
				sz_found += (size_t)UVADDR(0, 1, 0, 0, 0);
				continue;
			}

			const arch_pde_t *pdt = (arch_pde_t *)
				hn_tmpmap_early(
					UNPGADDR(pdpt[pdptx].address),
					sizeof(arch_pde_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);

			kfxx::deferred release_pde_guard([pdt]() noexcept {
				hn_tmpunmap_early((void *)pdt, PAGESIZE);
			});

			// Walk each PDE.
			for (uint16_t pdx =
					 (pml4x == PML4X(minaddr) &&
								 pdptx == PDPTX(minaddr)
							 ? PDX(minaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				if (sz_found >= size)
					break;

				char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				if (!(pdt[pdx].mask & PDE_P)) {
					if (!p_found)
						p_found = pdt_vaddr;
					sz_found += (size_t)UVADDR(0, 0, 1, 0, 0);
					continue;
				}

				const arch_pte_t *ptt = (arch_pte_t *)
					hn_tmpmap_early(
						UNPGADDR(pdt[pdx].address),
						sizeof(arch_pte_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);

				kfxx::deferred release_pte_guard([ptt]() noexcept {
					hn_tmpunmap_early((void *)ptt, PAGESIZE);
				});

				// Walk each PTE.
				for (uint16_t ptx =
						 (pml4x == PML4X(minaddr) &&
									 pdptx == PDPTX(minaddr) &&
									 pdx == PDX(minaddr)
								 ? PTX(minaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					if (sz_found >= size)
						break;

					char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					if (!(ptt[ptx].mask & PTE_P)) {
						if (!p_found)
							p_found = ptt_vaddr;
						sz_found += (size_t)UVADDR(0, 0, 0, 1, 0);
						continue;
					}
				}
			}
		}
	}

	if (sz_found >= size)
		return p_found;

	return nullptr;
}

void *kh_getmap(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	// io::irq_disable_lock irq_lock;

	kd_assert(ctxt);

	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;

	void *p_found = NULL;  // Pointer to the free area.
	size_t sz_found = 0;   // Size of free area found.

	// Walk PML4E.
	uint16_t pml4x = PML4X(vaddr);

	arch_pml4te_t *pml4te = &pml4t[pml4x];

	if (!(pml4te->mask & PML4E_P))
		return nullptr;

	arch_pdpte_t *pdpt;
	void *pdpt_paddr = UNPGADDR(pml4te->address);
	kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
		hn_tmpunmap_post((void *)pdpt, PAGESIZE);
	});
	if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
		pdpt = (arch_pdpte_t *)
			hn_tmpmap_post(
				pdpt_paddr,
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdpt_guard.release();

	// Walk PDPTE.
	uint16_t pdptx = PDPTX(vaddr);

	char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

	if (!(pdpt[pdptx].mask & PDPTE_P))
		return nullptr;

	const arch_pde_t *pdt;
	void *pdt_paddr = UNPGADDR(pdpt[pdptx].address);
	kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
		hn_tmpunmap_post((void *)pdt, PAGESIZE);
	});
	if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
		pdt = (arch_pde_t *)
			hn_tmpmap_post(
				UNPGADDR(pdpt[pdptx].address),
				sizeof(arch_pde_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdt_guard.release();

	// Walk PDE.
	uint16_t pdx = PDX(vaddr);

	char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

	if (!(pdt[pdx].mask & PDE_P))
		return nullptr;

	const arch_pte_t *ptt;
	kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
		hn_tmpunmap_post((void *)ptt, PAGESIZE);
	});
	if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(UNPGADDR(pdt[pdx].address)))) {
		ptt = (arch_pte_t *)
			hn_tmpmap_post(
				UNPGADDR(pdt[pdx].address),
				sizeof(arch_pte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_ptt_guard.release();

	// Walk PTE.
	uint16_t ptx = PTX(vaddr);

	char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

	if (!(ptt[ptx].mask & PTE_P)) {
		return nullptr;
	}

	if (pgaccess_out) {
		*pgaccess_out = MM_PAGE_READ;
		if (ptt[ptx].mask & PTE_P)
			*pgaccess_out |= MM_PAGE_MAPPED;
		if (ptt[ptx].mask & PTE_RW)
			*pgaccess_out |= MM_PAGE_WRITE;
		if (ptt[ptx].mask & PTE_U)
			*pgaccess_out |= MM_PAGE_USER;
		if (!ptt[ptx].xd)
			*pgaccess_out |= MM_PAGE_EXEC;
	}

	return UNPGADDR(ptt[ptx].address);
}

PBOS_NODISCARD void *hn_tmpmap_early(void *paddr, size_t size, uint16_t mask) {
	// kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpmap() requires interrupts disabled");
	kd_dbgcheck(
		size <= (KINITTMPMAP_SIZE),
		"Size of space to map is bigger than the size of KINITTMPMAP area");
	kd_dbgcheck(!PGOFF(paddr), "Address for hn_tmpmap must be page-aligned");
	kd_dbgcheck(!PGOFF(size), "Size for hn_tmpmap must be page-aligned");
	kd_dbgcheck(mask & PTE_P, "PTE_P must be set for hn_tmpmap");

	char *vaddr = nullptr;

	size_t sz_found = 0;
	for (uint16_t pml4x = PML4X(KINITTMPMAP_VBASE); pml4x < PML4X(KINITTMPMAP_VTOP + 1) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &mm_kernel_initial_pml4t[pml4x];

		kd_assert(pml4te->mask & PML4E_P);

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(KINITTMPMAP_VBASE) ? PDPTX(KINITTMPMAP_VBASE) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);

			if (pdpt_vaddr > (void *)KINITTMPMAP_VTOP)
				break;

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KBOTTOM_VBASE)) * 512 + pdptx];

			kd_assert(pdpte->mask & PDPTE_P);

			// Walk each PDE.
			for (uint16_t pdx =
					 (pml4x == PML4X(KINITTMPMAP_VBASE) &&
								 pdptx == PDPTX(KINITTMPMAP_VBASE)
							 ? PDX(KINITTMPMAP_VBASE)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);

				if (pdt_vaddr > (void *)KINITTMPMAP_VTOP)
					break;

				arch_pde_t *pde =
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KBOTTOM_VBASE)) * 512 +
										   pdx];
				kd_assert(pde->mask & PDE_P);

				// Walk each PTE.
				for (uint16_t ptx =
						 (pml4x == PML4X(KINITTMPMAP_VBASE) &&
									 pdptx == PDPTX(KINITTMPMAP_VBASE) &&
									 pdx == PDX(KINITTMPMAP_VBASE)
								 ? PTX(KINITTMPMAP_VBASE)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					if (sz_found >= size)
						goto alloc_succeeded;
					char *const ptt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);

					if (ptt_vaddr > (void *)KINITTMPMAP_VTOP)
						break;

					arch_pte_t *pte =
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KBOTTOM_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KBOTTOM_VBASE)) * 512 +
											   ptx];

					if (pte->mask & PTE_P) {
						sz_found = 0;
						vaddr = nullptr;
					} else {
						if (!vaddr)
							vaddr = ptt_vaddr;
						sz_found += PAGESIZE;
					}
				}
			}
		}
	}

	km_panic("Out of TMPMAP virtual memory");

alloc_succeeded:
	char *const addr_limit = vaddr + size;

	size_t off_paddr = 0;
	for (uint32_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit - PAGESIZE) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &mm_kernel_initial_pml4t[pml4x];
		kd_assert(pml4te->mask & PML4E_P);

		for (uint32_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KBOTTOM_VBASE)) * 512 + pdptx];
			kd_assert(pdpte->mask & PDPTE_P);

			for (uint32_t pdx =
					 (pml4x == PML4X(vaddr) &&
								 pdptx == PDPTX(vaddr)
							 ? PDX(vaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				arch_pde_t *pde =
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KBOTTOM_VBASE)) * 512 +
										   pdx];
				kd_assert(pde->mask & PDE_P);

				for (uint32_t ptx =
						 (pml4x == PML4X(vaddr) &&
									 pdptx == PDPTX(vaddr) &&
									 pdx == PDX(vaddr)
								 ? PTX(vaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const ptt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					arch_pte_t *pte =
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KBOTTOM_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KBOTTOM_VBASE)) * 512 +
											   ptx];
					kd_assert(!(pte->mask & PTE_P));

					pte->mask = mask;
					pte->address = PGROUNDDOWN(((char *)paddr) + off_paddr);

					arch_invlpg(ptt_vaddr);

					off_paddr += PAGESIZE;
				}
			}
		}
	}

	return vaddr;
}

void hn_tmpunmap_early(void *vaddr, size_t size) {
	// kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpunmap() requires interrupts disabled");
	kd_dbgcheck(!PGOFF(vaddr), "Address for hn_tmpunmap must be page-aligned");

	char *addr_limit = (char *)vaddr + size;

	size_t off_paddr = 0;
	for (uint32_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit - PAGESIZE) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &mm_kernel_initial_pml4t[pml4x];

		for (uint32_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KBOTTOM_VBASE)) * 512 + pdptx];

			for (uint32_t pdx =
					 (pml4x == PML4X(vaddr) &&
								 pdptx == PDPTX(vaddr)
							 ? PDX(vaddr)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);

				if (pdt_vaddr >= (void *)addr_limit)
					break;

				arch_pde_t *pde =
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KBOTTOM_VBASE)) * 512 +
										   pdx];

				for (uint32_t ptx =
						 (pml4x == PML4X(vaddr) &&
									 pdptx == PDPTX(vaddr) &&
									 pdx == PDX(vaddr)
								 ? PTX(vaddr)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const ptt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);

					if (ptt_vaddr >= (void *)addr_limit)
						break;

					arch_pte_t *pte =
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KBOTTOM_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KBOTTOM_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KBOTTOM_VBASE)) * 512 +
											   ptx];

					pte->mask &= ~PTE_P;

					arch_invlpg(ptt_vaddr);

					off_paddr += PAGESIZE;
				}
			}
		}
	}
}

PBOS_NODISCARD void *hn_tmpmap_post(void *paddr, size_t size, uint16_t mask) {
	hn_tmpmap_info_t *tmpmap_info = &hn_tmpmap_storage_ptr[ps_get_cur_cpuid()];
	// kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpmap() requires interrupts disabled");
	kd_dbgcheck(
		size <= (KINITTMPMAP_SIZE),
		"Size of space to map is bigger than the size of KINITTMPMAP area");
	kd_dbgcheck(!PGOFF(paddr), "Address for hn_tmpmap must be page-aligned");
	kd_dbgcheck(!PGOFF(size), "Size for hn_tmpmap must be page-aligned");
	kd_dbgcheck(mask & PTE_P, "PTE_P must be set for hn_tmpmap");

	char *vaddr = nullptr;

	size_t sz_found = 0;
	size_t idx_found_area;

	for (size_t i = 0; i < KINITTMPMAP_SIZE - size; i += PAGESIZE) {
		if (sz_found >= size)
			goto alloc_succeeded;
		char *const ptt_vaddr = ((char *)tmpmap_info->tmpmap_base) + i;

		if (ptt_vaddr > (void *)KINITTMPMAP_VTOP)
			break;

		arch_pte_t *pte = &tmpmap_info->tmpmap_pgtab_base[i / PAGESIZE];

		if ((pte->mask & ~PTE_P)) {
			sz_found = 0;
			vaddr = nullptr;
		} else {
			if (!vaddr)
				vaddr = ptt_vaddr;
			sz_found += PAGESIZE;
			idx_found_area = i;
		}
	}

	km_panic("Out of TMPMAP virtual memory");

alloc_succeeded:
	char *const addr_limit = vaddr + size;

	size_t off_paddr = 0;

	for (size_t i = 0; i < size; i += PAGESIZE) {
		char *const ptt_vaddr = vaddr + (idx_found_area + i) * PAGESIZE;

		arch_pte_t *pte = &tmpmap_info->tmpmap_pgtab_base[(i + idx_found_area) / PAGESIZE];

		kd_assert(!(pte->mask & PTE_P));

		pte->mask = mask;
		pte->address = PGROUNDDOWN(((char *)paddr) + off_paddr);

		arch_invlpg(ptt_vaddr);

		off_paddr += PAGESIZE;
	}

	return vaddr;
}

void hn_tmpunmap_post(void *vaddr, size_t size) {
	hn_tmpmap_info_t *tmpmap_info = &hn_tmpmap_storage_ptr[ps_get_cur_cpuid()];
	// kd_dbgcheck(hal_is_irq_disabled(), "hn_tmpunmap() requires interrupts disabled");
	kd_dbgcheck(!PGOFF(vaddr), "Address for hn_tmpunmap must be page-aligned");

	char *addr_limit = (char *)vaddr + size;

	for (size_t i = 0; i < size; i += PAGESIZE) {
		char *const ptt_vaddr = ((char *)vaddr) + i;

		if (ptt_vaddr > (void *)KINITTMPMAP_VTOP)
			break;

		arch_pte_t *pte = &tmpmap_info->tmpmap_pgtab_base[(((char *)vaddr) - (char *)tmpmap_info->tmpmap_base) / PAGESIZE + (i / PAGESIZE)];

		kd_assert(pte->mask & PTE_P);

		pte->mask = PTE_P;

		arch_invlpg(ptt_vaddr);
	}
}

void *hn_get_pgtab_paddr(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	// io::irq_disable_lock irq_lock;

	kd_assert(ctxt);

	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;

	void *p_found = NULL;  // Pointer to the free area.
	size_t sz_found = 0;   // Size of free area found.

	// Walk PML4E.
	uint16_t pml4x = PML4X(vaddr);

	arch_pml4te_t *pml4te = &pml4t[pml4x];

	if (!(pml4te->mask & PML4E_P))
		return nullptr;

	arch_pdpte_t *pdpt;
	void *pdpt_paddr = UNPGADDR(pml4te->address);
	kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
		hn_tmpunmap_post((void *)pdpt, PAGESIZE);
	});
	if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
		pdpt = (arch_pdpte_t *)
			hn_tmpmap_post(
				pdpt_paddr,
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdpt_guard.release();

	// Walk PDPTE.
	uint16_t pdptx = PDPTX(vaddr);

	char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

	if (!(pdpt[pdptx].mask & PDPTE_P))
		return nullptr;

	const arch_pde_t *pdt;
	void *pdt_paddr = UNPGADDR(pdpt[pdptx].address);
	kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
		hn_tmpunmap_post((void *)pdt, PAGESIZE);
	});
	if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
		pdt = (arch_pde_t *)
			hn_tmpmap_post(
				UNPGADDR(pdpt[pdptx].address),
				sizeof(arch_pde_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdt_guard.release();

	// Walk PDE.
	uint16_t pdx = PDX(vaddr);

	char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

	if (!(pdt[pdx].mask & PDE_P))
		return nullptr;

	return UNPGADDR(pdt[pdx].address);
}

PBOS_EXTERN_C_END

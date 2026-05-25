#include "vm.hh"
#include <pbos/kd/logger.h>
#include <pbos/ps/proc.h>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/mm/pgalloc.hh>

PBOS_EXTERN_C_BEGIN

static uint16_t hali_pgaccess_to_pgmask(mm_pgaccess_t access) {
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
	ki_pmad_t *pmad = ki_get_pmad(paddr);

	if ((!pmad) || (!pmad->direct_map_base))
		return nullptr;

	ptrdiff_t off = (((char *)paddr) - (char *)pmad->rb_value);
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

PBOS_NODISCARD uint8_t hali_mm_mmap_early(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	mm_pgaccess_t access,
	void *pdpte_paddr,
	void *pde_paddr,
	void *pte_paddr) {
	uint8_t result = 0;
	const uint8_t pgaccess = hali_pgaccess_to_pgmask(access);

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

		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P)) {
			result |= 0b100;
			*pml4te =
				ARCH_PML4TE_WITH_ADDR(
					ARCH_PML4TE_WITH_MASKS(*pml4te, PML4E_P | PML4E_RW),
					PGROUNDDOWN(pdpte_paddr));
			init_pdpt = true;
		}

		arch_pdpte_t *pdpt = (arch_pdpte_t *)
			hali_tmpmap_early(
				UNPGADDR(ARCH_PML4TE_ADDR(*pml4te)),
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);

		kfxx::deferred release_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_early((void *)pdpt, PAGESIZE);
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

			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P)) {
				result |= 0b010;
				pdpt[pdptx] =
					ARCH_PDPTE_WITH_ADDR(
						ARCH_PDPTE_WITH_MASKS(pdpt[pdptx], PDPTE_P | PDPTE_RW),
						PGROUNDDOWN(pde_paddr));
				init_pdt = true;
			}

			arch_pde_t *pdt = (arch_pde_t *)
				hali_tmpmap_early(
					UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx])),
					sizeof(arch_pde_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);

			kfxx::deferred release_pde_guard([&pdt]() noexcept {
				hali_tmpunmap_early((void *)pdt, PAGESIZE);
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

				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P)) {
					result |= 0b001;
					pdt[pdx] =
						ARCH_PDE_WITH_ADDR(
							ARCH_PDE_WITH_MASKS(pdt[pdx], PDE_P | PDE_RW),
							PGROUNDDOWN(pte_paddr));
					init_ptt = true;
				}

				arch_pte_t *ptt = (arch_pte_t *)
					hali_tmpmap_early(
						UNPGADDR(ARCH_PDE_ADDR(pdt[pdx])),
						sizeof(arch_pte_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);

				if (init_ptt)
					memset(ptt, 0, PAGESIZE);

				kfxx::deferred release_pte_guard([&ptt]() noexcept {
					hali_tmpunmap_early((void *)ptt, PAGESIZE);
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

					ptt[ptx] =
						ARCH_PTE_WITH_XD(
							ARCH_PTE_WITH_ADDR(
								ARCH_PTE_WITH_MASKS(ptt[ptx], pgaccess),
								PGROUNDDOWN((char *)paddr + sz_mapped)),
							!(access & MM_PAGE_EXEC));

					// Note we don't increase the page's reference count because the page will not be multiple-mapped.

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
	// io::LocalIrqLock irq_lock;

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
	uint8_t mask = hali_pgaccess_to_pgmask(access);

	kfxx::scope_guard unmmap_guard([ctxt, vaddr, size]() noexcept {
		kh_unmmap(ctxt, vaddr, size, 0);
	});

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &pml4t[pml4x];

		bool is_pml4te_allocated = false;
		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P)) {
			if (flags & MMAP_NO_PGTAB_ALLOC)
				km_panic("Missing PML4 table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
			// Allocate page table.
			void *t = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
			*pml4te =
				ARCH_PML4TE_WITH_XD(
					ARCH_PML4TE_WITH_ADDR(
						ARCH_PML4TE_WITH_MASKS(0, PML4E_P | PML4E_RW | PML4E_U),
						PGROUNDDOWN(t)),
					false);
			is_pml4te_allocated = true;
		}

		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hali_tmpmap_post(
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
			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P)) {
				if (flags & MMAP_NO_PGTAB_ALLOC)
					km_panic("Missing PDP table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
				// Allocate page table.
				void *t = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
				pdpt[pdptx] =
					ARCH_PDPTE_WITH_XD(
						ARCH_PDPTE_WITH_ADDR(
							ARCH_PDPTE_WITH_MASKS(0, PDPTE_P | PDPTE_RW | PDPTE_U),
							PGROUNDDOWN(t)),
						false);
				is_pdpte_allocated = true;
			}

			arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hali_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hali_tmpmap_post(
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
				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P)) {
					if (flags & MMAP_NO_PGTAB_ALLOC)
						km_panic("Missing page directory table entry with MMAP_NO_PGTAB_ALLOC, please report this bug.");
					// Allocate page table.
					void *t = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
					pdt[pdx] =
						ARCH_PDE_WITH_XD(
							ARCH_PDE_WITH_ADDR(
								ARCH_PDE_WITH_MASKS(0, PDE_P | PDE_RW | PDE_U),
								PGROUNDDOWN(t)),
							false);
					is_pde_allocated = true;
				}

				arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]));
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hali_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hali_tmpmap_post(
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

					if (ARCH_PTE_MASK(*pte) & PTE_P) {
						if (flags & MMAP_NO_REMAP)
							km_panic("Remapping address %p with MMAP_NO_REMAP", ptt_vaddr);

						void *addr = (void *)UNPGADDR(ARCH_PTE_ADDR(*pte));
						if (addr)
							mm_unpin_page(addr);
					} else {
					}

					if (!(flags & MMAP_NO_INC_RC)) {
						if (pi) {
							if (is_user_space)
								mm_ref_page(pi);
							else
								mm_pin_page(pi);
						}
					}

					*pte =
						ARCH_PTE_WITH_XD(
							ARCH_PTE_WITH_ADDR(
								ARCH_PTE_WITH_MASKS(*pte, mask),
								PGROUNDDOWN(pi)),
							!(access & MM_PAGE_EXEC));

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

#ifndef NDEBUG
	bool is_user_space = mm_is_user_space(vaddr);

	if (is_user_space !=
		mm_is_user_space((void *)(((uintptr_t)vaddr) + (size - 1))))
		km_panic("Cannot map across user and kernel spaces");
#endif

	// io::LocalIrqLock irq_lock;
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;
	void *const addr_limit = (char *)vaddr + size;
	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	bool is_cur_pgtab = ctxt == mm_get_cur_context();

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit); ++pml4x) {
		arch_pml4te_t *pml4te = &pml4t[pml4x];

		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P))
			continue;

		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hali_tmpmap_post(
					UNPGADDR(ARCH_PML4TE_ADDR(*pml4te)),
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

			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P))
				continue;

			arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hali_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hali_tmpmap_post(
						UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx])),
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

				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P))
					continue;

				arch_pte_t *ptt;
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hali_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(pdt_paddr))) {
					ptt = (arch_pte_t *)
						hali_tmpmap_post(
							UNPGADDR(ARCH_PDPTE_ADDR(pdt[pdx])),
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

					if (ARCH_PTE_MASK(*pte) & PTE_P) {
						if (flags & MMAP_NO_REMAP)
							km_panic("Remapping address %p with MMAP_NO_REMAP", ptt_vaddr);

						if (!(flags & MMAP_NO_INC_RC)) {
							void *addr = (void *)UNPGADDR(ARCH_PTE_ADDR(*pte));
							if (addr) {
								if (is_user_space)
									mm_unref_page(addr);
								else
									mm_unpin_page(addr);
							}
						}
					}

					*pte = ARCH_PTE_WITH_MASKS(0, 0);

					if (is_cur_pgtab)
						arch_invlpg(ptt_vaddr);
				}

				// Check if the page table is unneeded.
				for (uint16_t i = 0; i < PTX_MAX; ++i) {
					if (ARCH_PTE_MASK(ptt[i]) & PTE_P)
						goto skip_release_pt;
				}

				{
					pgaddr_t pde_addr = ARCH_PDE_ADDR(pdt[pdx]);
					pdt[pdx] = ARCH_PDE_WITH_MASKS(0, 0);
					mm_unpin_page(UNPGADDR(pde_addr));
				}

			skip_release_pt:;
			}

			// Check if the page directory is unneeded.
			for (uint16_t i = 0; i < PDX_MAX; ++i) {
				if (ARCH_PDE_MASK(pdt[i]) & PDE_P)
					goto skip_release_pd;
			}

			{
				pgaddr_t pdpte_addr = ARCH_PDE_ADDR(pdpt[pdptx]);
				pdpt[pdptx] = ARCH_PDE_WITH_MASKS(0, 0);
				mm_unpin_page(UNPGADDR(pdpte_addr));
			}

		skip_release_pd:;
		}

		// Check if the PML4T is unneeded.
		for (uint16_t i = 0; i < PDPTX_MAX; ++i) {
			if (ARCH_PDPTE_MASK(pdpt[i]) & PDPTE_P)
				goto skip_release_pdp;
		}

		{
			pgaddr_t pml4te_addr = ARCH_PDE_ADDR(pml4te[pml4x]);
			pml4te[pml4x] = ARCH_PDE_WITH_MASKS(0, 0);
			mm_unpin_page(UNPGADDR(pml4te_addr));
		}

	skip_release_pdp:;
	}
}

void kh_walk_pgtab(mm_context_t *ctxt, void *vaddr, size_t size, kh_pgtab_walker walker, void *user_data) {
#ifndef NDEBUG
	bool is_user_space = mm_is_user_space(vaddr);

	if (is_user_space !=
		mm_is_user_space((void *)(((uintptr_t)vaddr) + (size - 1))))
		km_panic("Cannot walk across user and kernel spaces");
#endif

	void *const addr_limit = (char *)vaddr + size;
	const arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;
	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		const arch_pml4te_t *pml4te = &pml4t[pml4x];

		bool is_pml4te_allocated = false;
		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P)) {
			continue;
		}

		const arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hali_tmpmap_post(
					pdpt_paddr,
					sizeof(arch_pdpte_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);
		} else
			release_tmpmap_pdpt_guard.release();

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			bool is_pdpte_allocated = false;
			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P)) {
				continue;
			}

			const arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hali_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hali_tmpmap_post(
						pdt_paddr,
						sizeof(arch_pde_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);
			} else
				release_tmpmap_pdt_guard.release();

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
				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P)) {
					continue;
				}

				const arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]));
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hali_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hali_tmpmap_post(
							ptt_paddr,
							sizeof(arch_pte_t) * (PTX_MAX + 1),
							PTE_P | PTE_RW);
				} else
					release_tmpmap_ptt_guard.release();

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

					const arch_pte_t *pte = &ptt[ptx];

					mm_pgaccess_t pgaccess = MM_PAGE_READ;
					uint64_t pte_mask = ARCH_PTE_MASK(ptt[ptx]);
					if (pte_mask & PTE_P)
						pgaccess |= MM_PAGE_MAPPED;
					if (pte_mask & PTE_RW)
						pgaccess |= MM_PAGE_WRITE;
					if (pte_mask & PTE_U)
						pgaccess |= MM_PAGE_USER;
					if (!ARCH_PTE_XD(ptt[ptx]))
						pgaccess |= MM_PAGE_EXEC;

					if (walker(ptt_vaddr, (void *)UNPGADDR(ARCH_PTE_ADDR(*pte)), pgaccess, user_data) == KF_CONTROL_FLOW_BREAK)
						return;
				}
			}
		}
	}
}

void kh_set_page_access(
	mm_context_t *ctxt,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
	// io::LocalIrqLock irq_lock;

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

	uint8_t mask = hali_pgaccess_to_pgmask(access);

	mask |= PTE_P;

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(vaddr); pml4x < PML4X(addr_limit) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &pml4t[pml4x];

		bool is_pml4te_allocated = false;
		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P))
			km_panic("Missing PML4 table entry in %s, please report this bug.", __func__);

		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hali_tmpmap_post(
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
			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P))
				km_panic("Missing PDP table entry with %s, please report this bug.", __func__);

			arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hali_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hali_tmpmap_post(
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
				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P))
					km_panic("Missing page directory table entry with %s, please report this bug.", __func__);

				arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]));
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hali_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hali_tmpmap_post(
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

					if (!(ARCH_PTE_MASK(*pte) & PTE_P))
						km_panic("%s with page that is not present", __func__);

					*pte =
						ARCH_PTE_WITH_XD(
							ARCH_PTE_WITH_MASKS(*pte, mask),
							!(access & MM_PAGE_EXEC));

					if (is_cur_pgtab)
						arch_invlpg(ptt_vaddr);
				}
			}
		}
	}
}

void *kh_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags) {
	// io::LocalIrqLock irq_lock;
	void *const addr_limit = (void *)(PGCEIL((char *)maxaddr) - PGCEIL(size) - 1);
	uintptr_t addr_prefix = ADDR_PREFIX(minaddr);
	kd_dbgcheck(
		addr_prefix == ADDR_PREFIX(maxaddr),
		"The address prefix of the minimum address and the maximum address does not match");
	arch_pml4te_t *pml4t = (arch_pml4te_t *)context->page_table;

	void *p_found = nullptr;  // Pointer to the free area.
	size_t sz_found = 0;	  // Size of free area found.

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(minaddr); pml4x < PML4X(maxaddr) + 1; ++pml4x) {
		// Return if the size is enough.
		if (sz_found >= size)
			break;

		arch_pml4te_t *pml4te = &pml4t[pml4x];

		// Skip if does not present
		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P)) {
			if (!p_found) {
				if ((p_found = (void *)(addr_prefix | (uintptr_t)UVADDR(pml4x, 0, 0, 0, 0))) < minaddr)
					p_found = (void *)minaddr;
			}
			sz_found += (size_t)UVADDR(1, 0, 0, 0, 0);
			continue;
		}

		// io::LocalIrqLock irq_lock;
		arch_pdpte_t *pdpt;
		void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
		kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
			hali_tmpunmap_post((void *)pdpt, PAGESIZE);
		});
		if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
			pdpt = (arch_pdpte_t *)
				hali_tmpmap_post(
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

			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P)) {
				if (!p_found) {
					if ((p_found = pdpt_vaddr) < minaddr)
						p_found = (void *)minaddr;
				}
				sz_found += (size_t)UVADDR(0, 1, 0, 0, 0);
				continue;
			}

			const arch_pde_t *pdt;
			void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
			kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
				hali_tmpunmap_post((void *)pdt, PAGESIZE);
			});
			if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
				pdt = (arch_pde_t *)
					hali_tmpmap_post(
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

				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P)) {
					if (!p_found) {
						if ((p_found = pdt_vaddr) < minaddr)
							p_found = (void *)minaddr;
					}
					sz_found += (size_t)UVADDR(0, 0, 1, 0, 0);
					continue;
				}

				const arch_pte_t *ptt;
				void *ptt_paddr = UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]));
				kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
					hali_tmpunmap_post((void *)ptt, PAGESIZE);
				});
				if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(ptt_paddr))) {
					ptt = (arch_pte_t *)
						hali_tmpmap_post(
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

					if (!(ARCH_PTE_MASK(ptt[ptx]) & PTE_P)) {
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

	void *p_found = nullptr;  // Pointer to the free area.
	size_t sz_found = 0;	  // Size of free area found.

	// Walk each PML4E.
	for (uint16_t pml4x = PML4X(minaddr); pml4x < PML4X(maxaddr) + 1; ++pml4x) {
		// Return if the size is enough.
		if (sz_found >= size)
			break;

		arch_pml4te_t *pml4te = &pml4t[pml4x];

		// Skip if does not present
		if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P)) {
			if (!p_found)
				p_found = (void *)(addr_prefix | (uintptr_t)UVADDR(pml4x, 0, 0, 0, 0));
			sz_found += (size_t)UVADDR(1, 0, 0, 0, 0);
			continue;
		}

		// io::LocalIrqLock irq_lock;
		const arch_pdpte_t *pdpt = (arch_pdpte_t *)
			hali_tmpmap_early(
				UNPGADDR(ARCH_PML4TE_ADDR(*pml4te)),
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);

		kfxx::deferred release_pdpt_guard([pdpt]() noexcept {
			hali_tmpunmap_early((void *)pdpt, PAGESIZE);
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

			if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P)) {
				if (!p_found)
					p_found = pdpt_vaddr;
				sz_found += (size_t)UVADDR(0, 1, 0, 0, 0);
				continue;
			}

			const arch_pde_t *pdt = (arch_pde_t *)
				hali_tmpmap_early(
					UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx])),
					sizeof(arch_pde_t) * (PTX_MAX + 1),
					PTE_P | PTE_RW);

			kfxx::deferred release_pde_guard([pdt]() noexcept {
				hali_tmpunmap_early((void *)pdt, PAGESIZE);
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

				if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P)) {
					if (!p_found)
						p_found = pdt_vaddr;
					sz_found += (size_t)UVADDR(0, 0, 1, 0, 0);
					continue;
				}

				const arch_pte_t *ptt = (arch_pte_t *)
					hali_tmpmap_early(
						UNPGADDR(ARCH_PDE_ADDR(pdt[pdx])),
						sizeof(arch_pte_t) * (PTX_MAX + 1),
						PTE_P | PTE_RW);

				kfxx::deferred release_pte_guard([ptt]() noexcept {
					hali_tmpunmap_early((void *)ptt, PAGESIZE);
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

					if (!(ARCH_PTE_MASK(ptt[ptx]) & PTE_P)) {
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
	// io::LocalIrqLock irq_lock;
	if (pgaccess_out)
		*pgaccess_out = 0;

	kd_assert(ctxt);

	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;

	void *p_found = nullptr;  // Pointer to the free area.
	size_t sz_found = 0;	  // Size of free area found.

	// Walk PML4E.
	uint16_t pml4x = PML4X(vaddr);

	arch_pml4te_t *pml4te = &pml4t[pml4x];

	if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P))
		return nullptr;

	arch_pdpte_t *pdpt;
	void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
	kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
		hali_tmpunmap_post((void *)pdpt, PAGESIZE);
	});
	if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
		pdpt = (arch_pdpte_t *)
			hali_tmpmap_post(
				pdpt_paddr,
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdpt_guard.release();

	// Walk PDPTE.
	uint16_t pdptx = PDPTX(vaddr);

	char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

	if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P))
		return nullptr;

	const arch_pde_t *pdt;
	void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
	kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
		hali_tmpunmap_post((void *)pdt, PAGESIZE);
	});
	if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
		pdt = (arch_pde_t *)
			hali_tmpmap_post(
				UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx])),
				sizeof(arch_pde_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdt_guard.release();

	// Walk PDE.
	uint16_t pdx = PDX(vaddr);

	char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

	if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P))
		return nullptr;

	const arch_pte_t *ptt;
	kfxx::scope_guard release_tmpmap_ptt_guard([&ptt]() noexcept {
		hali_tmpunmap_post((void *)ptt, PAGESIZE);
	});
	if (!(ptt = (arch_pte_t *)kh_get_direct_mmap(UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]))))) {
		ptt = (arch_pte_t *)
			hali_tmpmap_post(
				UNPGADDR(ARCH_PDE_ADDR(pdt[pdx])),
				sizeof(arch_pte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_ptt_guard.release();

	// Walk PTE.
	uint16_t ptx = PTX(vaddr);

	char *const ptt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, ptx, 0));

	if (!(ARCH_PTE_MASK(ptt[ptx]) & PTE_P)) {
		return nullptr;
	}

	if (pgaccess_out) {
		*pgaccess_out = MM_PAGE_READ;
		uint64_t pte_mask = ARCH_PTE_MASK(ptt[ptx]);
		if (pte_mask & PTE_P)
			*pgaccess_out |= MM_PAGE_MAPPED;
		if (pte_mask & PTE_RW)
			*pgaccess_out |= MM_PAGE_WRITE;
		if (pte_mask & PTE_U)
			*pgaccess_out |= MM_PAGE_USER;
		if (!ARCH_PTE_XD(ptt[ptx]))
			*pgaccess_out |= MM_PAGE_EXEC;
	}

	return UNPGADDR(ARCH_PTE_ADDR(ptt[ptx]));
}

PBOS_NODISCARD void *hali_tmpmap_early(void *paddr, size_t size, uint16_t mask) {
	// kd_dbgcheck(hal_is_irq_disabled(), "hali_tmpmap() requires interrupts disabled");
	kd_dbgcheck(
		size <= (KINITTMPMAP_SIZE),
		"Size of space to map is bigger than the size of KINITTMPMAP area");
	kd_dbgcheck(!PGOFF(paddr), "Address for hali_tmpmap must be page-aligned");
	kd_dbgcheck(!PGOFF(size), "Size for hali_tmpmap must be page-aligned");
	kd_dbgcheck(mask & PTE_P, "PTE_P must be set for hali_tmpmap");

	char *vaddr = nullptr;

	size_t sz_found = 0;
	for (uint16_t pml4x = PML4X(mm_kernel_init_tmpmap_area); pml4x < PML4X((&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]) + 1) + 1; ++pml4x) {
		arch_pml4te_t *pml4te = &mm_kernel_initial_pml4t[pml4x];

		kd_assert(ARCH_PML4TE_MASK(*pml4te) & PML4E_P);

		// Walk each PDPTE.
		for (uint16_t pdptx = (pml4x == PML4X(mm_kernel_init_tmpmap_area) ? PDPTX(mm_kernel_init_tmpmap_area) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);

			if (pdpt_vaddr > (void *)(&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]))
				break;

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KERNEL_VBASE)) * 512 + pdptx];

			kd_assert(ARCH_PDPTE_MASK(*pdpte) & PDPTE_P);

			// Walk each PDE.
			for (uint16_t pdx =
					 (pml4x == PML4X(mm_kernel_init_tmpmap_area) &&
								 pdptx == PDPTX(mm_kernel_init_tmpmap_area)
							 ? PDX(mm_kernel_init_tmpmap_area)
							 : 0);
				pdx < PDX_MAX + 1;
				++pdx) {
				char *const pdt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);

				if (pdt_vaddr > (void *)(&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]))
					break;

				arch_pde_t *pde =
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KERNEL_VBASE)) * 512 +
										   pdx];
				kd_assert(ARCH_PDE_MASK(*pde) & PDE_P);

				// Walk each PTE.
				for (uint16_t ptx =
						 (pml4x == PML4X(mm_kernel_init_tmpmap_area) &&
									 pdptx == PDPTX(mm_kernel_init_tmpmap_area) &&
									 pdx == PDX(mm_kernel_init_tmpmap_area)
								 ? PTX(mm_kernel_init_tmpmap_area)
								 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					if (sz_found >= size)
						goto alloc_succeeded;
					char *const ptt_vaddr = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);

					if (ptt_vaddr > (void *)(&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]))
						break;

					arch_pte_t *pte =
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KERNEL_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KERNEL_VBASE)) * 512 +
											   ptx];

					if (ARCH_PTE_MASK(*pte) & PTE_P) {
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

		for (uint32_t pdptx = (pml4x == PML4X(vaddr) ? PDPTX(vaddr) : 0);
			pdptx < PDPTX_MAX + 1;
			++pdptx) {
			char *const pdpt_vaddr = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);

			if (pdpt_vaddr >= (void *)addr_limit)
				break;

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KERNEL_VBASE)) * 512 + pdptx];

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
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KERNEL_VBASE)) * 512 +
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
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KERNEL_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KERNEL_VBASE)) * 512 +
											   ptx];

					*pte =
						ARCH_PTE_WITH_ADDR(
							ARCH_PTE_WITH_MASKS(*pte, mask),
							PGROUNDDOWN(((char *)paddr) + off_paddr));

					arch_invlpg(ptt_vaddr);

					off_paddr += PAGESIZE;
				}
			}
		}
	}

	return vaddr;
}

void hali_tmpunmap_early(void *vaddr, size_t size) {
	// kd_dbgcheck(hal_is_irq_disabled(), "hali_tmpunmap() requires interrupts disabled");
	kd_dbgcheck(!PGOFF(vaddr), "Address for hali_tmpunmap must be page-aligned");

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

			arch_pdpte_t *pdpte = &mm_kernel_initial_pdpt[(PML4X(pdpt_vaddr) - PML4X(KERNEL_VBASE)) * 512 + pdptx];

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
					&mm_kernel_initial_pdt[(PML4X(pdt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512) +
										   (PDPTX(pdt_vaddr) - PDPTX(KERNEL_VBASE)) * 512 +
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
						&mm_kernel_initial_ptt[(PML4X(ptt_vaddr) - PML4X(KERNEL_VBASE)) * (512 * 512 * 512) +
											   (PDPTX(ptt_vaddr) - PDPTX(KERNEL_VBASE)) * (512 * 512) +
											   (PDX(ptt_vaddr) - PDX(KERNEL_VBASE)) * 512 +
											   ptx];

					*pte = ARCH_PTE_WITH_MASKS(0, 0);

					arch_invlpg(ptt_vaddr);

					off_paddr += PAGESIZE;
				}
			}
		}
	}
}

PBOS_NODISCARD void *hali_tmpmap_post(void *paddr, size_t size, uint16_t mask) {
	hali_tmpmap_info_t *tmpmap_info = &hali_tmpmap_storage_ptr[ps_get_cur_cpuid()];
	// kd_dbgcheck(hal_is_irq_disabled(), "hali_tmpmap() requires interrupts disabled");
	kd_dbgcheck(
		size <= (KINITTMPMAP_SIZE),
		"Size of space to map is bigger than the size of KINITTMPMAP area");
	kd_dbgcheck(!PGOFF(paddr), "Address for hali_tmpmap must be page-aligned");
	kd_dbgcheck(!PGOFF(size), "Size for hali_tmpmap must be page-aligned");
	kd_dbgcheck(mask & PTE_P, "PTE_P must be set for hali_tmpmap");

	char *vaddr = nullptr;

	size_t sz_found = 0;
	size_t idx_found_area;

	for (size_t i = 0; i < KINITTMPMAP_SIZE - size; i += PAGESIZE) {
		if (sz_found >= size)
			goto alloc_succeeded;
		char *const ptt_vaddr = ((char *)tmpmap_info->tmpmap_base) + i;

		if (ptt_vaddr > (void *)(&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]))
			break;

		arch_pte_t *pte = &tmpmap_info->tmpmap_pgtab_base[i / PAGESIZE];

		if ((ARCH_PTE_MASK(*pte) & ~PTE_P)) {
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

		kd_assert(!(ARCH_PTE_MASK(*pte) & PTE_P));

		*pte =
			ARCH_PTE_WITH_ADDR(
				ARCH_PTE_WITH_MASKS(
					*pte, mask),
				PGROUNDDOWN(((char *)paddr) + off_paddr));

		arch_invlpg(ptt_vaddr);

		off_paddr += PAGESIZE;
	}

	return vaddr;
}

void hali_tmpunmap_post(void *vaddr, size_t size) {
	hali_tmpmap_info_t *tmpmap_info = &hali_tmpmap_storage_ptr[ps_get_cur_cpuid()];
	// kd_dbgcheck(hal_is_irq_disabled(), "hali_tmpunmap() requires interrupts disabled");
	kd_dbgcheck(!PGOFF(vaddr), "Address for hali_tmpunmap must be page-aligned");

	char *addr_limit = (char *)vaddr + size;

	for (size_t i = 0; i < size; i += PAGESIZE) {
		char *const ptt_vaddr = ((char *)vaddr) + i;

		if (ptt_vaddr > (void *)(&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)]))
			break;

		arch_pte_t *pte = &tmpmap_info->tmpmap_pgtab_base[(((char *)vaddr) - (char *)tmpmap_info->tmpmap_base) / PAGESIZE + (i / PAGESIZE)];

		kd_assert(ARCH_PTE_MASK(*pte) & PTE_P);

		*pte = ARCH_PTE_WITH_MASKS(*pte, PTE_P);

		arch_invlpg(ptt_vaddr);
	}
}

void *hali_get_pgtab_paddr(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	// io::LocalIrqLock irq_lock;

	kd_assert(ctxt);

	uintptr_t addr_prefix = ADDR_PREFIX(vaddr);
	arch_pml4te_t *pml4t = (arch_pml4te_t *)ctxt->page_table;

	void *p_found = nullptr;  // Pointer to the free area.
	size_t sz_found = 0;	  // Size of free area found.

	// Walk PML4E.
	uint16_t pml4x = PML4X(vaddr);

	arch_pml4te_t *pml4te = &pml4t[pml4x];

	if (!(ARCH_PML4TE_MASK(*pml4te) & PML4E_P))
		return nullptr;

	arch_pdpte_t *pdpt;
	void *pdpt_paddr = UNPGADDR(ARCH_PML4TE_ADDR(*pml4te));
	kfxx::scope_guard release_tmpmap_pdpt_guard([&pdpt]() noexcept {
		hali_tmpunmap_post((void *)pdpt, PAGESIZE);
	});
	if (!(pdpt = (arch_pdpte_t *)kh_get_direct_mmap(pdpt_paddr))) {
		pdpt = (arch_pdpte_t *)
			hali_tmpmap_post(
				pdpt_paddr,
				sizeof(arch_pdpte_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdpt_guard.release();

	// Walk PDPTE.
	uint16_t pdptx = PDPTX(vaddr);

	char *const pdpt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, 0, 0, 0));

	if (!(ARCH_PDPTE_MASK(pdpt[pdptx]) & PDPTE_P))
		return nullptr;

	const arch_pde_t *pdt;
	void *pdt_paddr = UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx]));
	kfxx::scope_guard release_tmpmap_pdt_guard([&pdt]() noexcept {
		hali_tmpunmap_post((void *)pdt, PAGESIZE);
	});
	if (!(pdt = (arch_pde_t *)kh_get_direct_mmap(pdt_paddr))) {
		pdt = (arch_pde_t *)
			hali_tmpmap_post(
				UNPGADDR(ARCH_PDPTE_ADDR(pdpt[pdptx])),
				sizeof(arch_pde_t) * (PTX_MAX + 1),
				PTE_P | PTE_RW);
	} else
		release_tmpmap_pdt_guard.release();

	// Walk PDE.
	uint16_t pdx = PDX(vaddr);

	char *const pdt_vaddr = (char *)(addr_prefix | (uintptr_t)UVADDR(pml4x, pdptx, pdx, 0, 0));

	if (!(ARCH_PDE_MASK(pdt[pdx]) & PDE_P))
		return nullptr;

	return UNPGADDR(ARCH_PDE_ADDR(pdt[pdx]));
}

PBOS_EXTERN_C_END

#include <pbos/km/logger.h>
#include <pbos/km/proc.h>
#include "../mm.h"

hn_kgdt_t hn_kgdt;
hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1] = {
	{ .attribs = { .base = 0, .len = 0, .maxord = 0, .type = KN_PMEM_END }, .madpools = { 0 } }
};

size_t hn_tss_storage_num;
arch_tss_t *hn_tss_storage_ptr;
char **hn_tss_stacks;

FASTCALL_DECL(static void, hn_push_pmad(hn_pmad_t *pmad));
static void hn_init_gdt();
static void hn_mm_init_paging();
static void hn_mm_init_pmadlist();
static void hn_mm_init_areas();
static void hn_init_tss();

void hn_mm_init() {
	mm_kernel_context->pdt = hn_kernel_pdt;

	hn_init_gdt();
	hn_mm_init_pmadlist();
	hn_mm_init_paging();
	hn_mm_init_areas();

	kima_init();

	ps_eu_num = 1;
	if (!(mm_current_contexts = mm_kmalloc(ps_eu_num * sizeof(mm_context_t *)))) {
		km_panic("Unable to allocate memory context for all CPUs");
	}

	for (ps_euid_t i = 0; i < ps_eu_num; ++i) {
		mm_current_contexts[i] = mm_kernel_context;
	}

	hn_init_tss();

	kdprintf("Initialized memory manager\n");
}

static void hn_init_tss() {
	hn_tss_storage_num = 1;
	hn_tss_storage_ptr = mm_kmalloc(hn_tss_storage_num * sizeof(arch_tss_t));
	if (!hn_tss_storage_ptr) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	memset(hn_tss_storage_ptr, 0, hn_tss_storage_num * sizeof(arch_tss_t));

	hn_tss_stacks = mm_kmalloc(hn_tss_storage_num * sizeof(char *));
	if (!hn_tss_stacks) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	for (size_t i = 0; i < hn_tss_storage_num; ++i) {
		if (!(hn_tss_stacks[i] = mm_kmalloc(1024 * 1024 * 2))) {
			km_panic("Unable to allocate memory for TSS stacks");
		}

		hn_tss_storage_ptr[i].ss0 = SELECTOR_KDATA;
		hn_tss_storage_ptr[i].esp0 = ((uint32_t)hn_tss_stacks[i]) + 1024 * 1024 * 2;
	}

	hn_kgdt.tss_desc =
		GDTDESC(((uintptr_t)hn_tss_storage_ptr), hn_tss_storage_num * sizeof(arch_tss_t), GDT_AB_P | GDT_AB_DPL(0) | GDT_SYSTYPE_TSS32, 0);

	arch_lgdt(&hn_kgdt, sizeof(hn_kgdt) / sizeof(arch_gdt_desc_t));
	arch_ltr(SELECTOR_TSS);

	kdprintf("Initialized TSS\n");
}

static void hn_mm_init_areas() {
	PMAD_FOREACH(i) {
		if (i->attribs.type != KN_PMEM_AVAILABLE)
			continue;

		pgaddr_t pooladdr_cur = i->attribs.base;
		pgaddr_t pooladdr_last = NULLPG;
		// Create MAD pages for each order.
		for (uint16_t j = 0; j <= i->attribs.maxord; ++j) {
			pgaddr_t addr = i->attribs.base;

			while (true) {
				i->madpools[j] = pooladdr_cur;
				pgaddr_t vaddr = hn_tmpmap(pooladdr_cur, 1, PTE_P | PTE_RW);
				hn_madpool_t *newpool = UNPGADDR(vaddr);

				memset(newpool->descs, 0, sizeof(newpool->descs));
				newpool->next = pooladdr_last;

				for (size_t k = 0; k < ARRAYLEN(newpool->descs); ++k) {
					if (addr >= i->attribs.base + i->attribs.len)
						break;

					hn_mad_t *cur_mad = &newpool->descs[k];

					cur_mad->flags |= MAD_P;
					cur_mad->pgaddr = addr;
					cur_mad->type = MAD_ALLOC_FREE;
					cur_mad->ref_count = 0;

					addr += MM_BLKPGSIZE(j);
				}

				pooladdr_last = pooladdr_cur;
				pooladdr_cur += 1;

				hn_tmpunmap(vaddr);

				if (addr >= i->attribs.base + i->attribs.len)
					break;
			}
		}
		for (pgaddr_t j = i->attribs.base; j < pooladdr_cur; ++j) {
			hn_set_pgblk_used(j, MAD_ALLOC_KERNEL, 0);
		}
	}

	PMAD_FOREACH(i) {
		if (i->attribs.type == KN_PMEM_AVAILABLE)
			continue;

		pgaddr_t pooladdr_last = NULLPG;
		// Create MAD pages for each order.
		for (uint16_t j = 0; j <= i->attribs.maxord; ++j) {
			pgaddr_t addr = i->attribs.base;

			while (true) {
				void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, 0);
				assert(paddr);
				i->madpools[j] = PGROUNDDOWN(paddr);
				pgaddr_t vaddr = hn_tmpmap(PGROUNDDOWN(paddr), 1, PTE_P | PTE_RW);
				hn_madpool_t *newpool = UNPGADDR(vaddr);

				memset(newpool->descs, 0, sizeof(newpool->descs));
				newpool->next = pooladdr_last;

				for (size_t k = 0; k < ARRAYLEN(newpool->descs); ++k) {
					if (addr >= i->attribs.base + i->attribs.len)
						break;

					hn_mad_t *cur_mad = &newpool->descs[k];

					cur_mad->flags |= MAD_P;
					cur_mad->pgaddr = addr;
					cur_mad->type = MAD_ALLOC_FREE;
					cur_mad->ref_count = 0;

					addr += MM_BLKPGSIZE(j);
				}

				pooladdr_last = PGROUNDDOWN(paddr);

				hn_tmpunmap(vaddr);

				if (addr >= i->attribs.base + i->attribs.len)
					break;
			}
		}
	}

	kdprintf("Initialized memory areas\n");
}

///
/// @brief Initialize and load GDT.
///
static void hn_init_gdt() {
	// NULL descriptor.
	hn_kgdt.null_desc = GDTDESC(0, 0, 0, 0);

	// Kernel mode descriptors.
	hn_kgdt.kcode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_EX, GDT_FL_DB | GDT_FL_GR);
	hn_kgdt.kdata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// User mode descriptors.
	hn_kgdt.ucode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_DC | GDT_AB_EX, GDT_FL_DB | GDT_FL_GR);
	hn_kgdt.udata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// TSS is a stub, we have to reload the GDT later.
	hn_kgdt.tss_desc =
		GDTDESC(0, 0xfffff, 0, 0);

	arch_lgdt(&hn_kgdt, sizeof(hn_kgdt) / sizeof(arch_gdt_desc_t));

	arch_loadds(SELECTOR_KDATA);
	arch_loades(SELECTOR_KDATA);

	// stub, the initial CPU always has EUID 0
	arch_loadfs(0);

	arch_loadgs(SELECTOR_KDATA);
	arch_loadss(SELECTOR_KDATA);
	arch_loadcs(SELECTOR_KCODE);

	kdprintf("Initialized GDT\n");
}

///
/// @brief Scan and push PMADs.
///
static void hn_mm_init_pmadlist() {
	for (uint16_t i = 0; i < ARCH_MMAP_MAX; ++i) {
		arch_mmap_entry_t *entry = &(ARCH_KARGS_PTR->mmaps[i]);
		if (entry->type == ARCH_MEM_END)
			break;

		hn_pmad_t pmad = { 0 };

		const uint32_t entry_max = entry->base + (entry->size - 1);

		switch (entry->type) {
			case ARCH_MEM_AVAILABLE:
				//
				// All available memory areas under the kernel should be reserved for hardwares.
				//
				if (entry->base < INIT_CRITICAL_PBASE) {
					if (entry_max >= INIT_CRITICAL_PBASE) {
						pmad.attribs.base = PGROUNDDOWN(entry->base);
						pmad.attribs.len = PGROUNDUP(INIT_CRITICAL_PBASE - entry->base);
						pmad.attribs.type = KN_PMEM_HARDWARE;
						hn_push_pmad(&pmad);

						pmad.attribs.base = PGROUNDDOWN(INIT_CRITICAL_PBASE);
						pmad.attribs.len = PGROUNDUP(INIT_CRITICAL_SIZE);
						pmad.attribs.type = KN_PMEM_CRITICAL;
						hn_push_pmad(&pmad);

						if (entry_max > INIT_CRITICAL_PTOP) {
							pmad.attribs.base = PGROUNDDOWN(INIT_CRITICAL_PTOP + 1);
							pmad.attribs.len = PGROUNDUP((entry_max + 1) - INIT_CRITICAL_PTOP);
							pmad.attribs.type = KN_PMEM_AVAILABLE;
						}
					} else {
						pmad.attribs.base = PGROUNDDOWN(entry->base);
						pmad.attribs.len = PGROUNDUP(entry->size + 1);
						pmad.attribs.type = KN_PMEM_HARDWARE;
					}
				} else {
					// If the area's base address is in kernel area:
					if (entry->base <= INIT_CRITICAL_PTOP) {
						// If the area is not in the range of kernel area completely:
						if (entry_max > INIT_CRITICAL_PTOP) {
							// Push the lower part and prepare to push the higher part at the loop end.
							pmad.attribs.base = PGROUNDDOWN(entry->base);
							pmad.attribs.len = PGROUNDUP((INIT_CRITICAL_PTOP - entry->base) + 1);
							pmad.attribs.type = KN_PMEM_CRITICAL;
							hn_push_pmad(&pmad);

							pmad.attribs.base = PGROUNDDOWN(INIT_CRITICAL_PTOP) + 1;
							pmad.attribs.len = PGROUNDUP((entry_max + 1) - INIT_CRITICAL_PTOP);
							pmad.attribs.type = KN_PMEM_AVAILABLE;
						}
						// If not, the area is completely in kernel area.
						else {
							pmad.attribs.base = PGROUNDDOWN(entry->base);
							pmad.attribs.len = PGROUNDUP(entry->size);
							pmad.attribs.type = KN_PMEM_CRITICAL;
						}
					}
					// If not, it is is higher than the kernel area.
					else {
						pmad.attribs.base = PGROUNDDOWN(entry->base);
						pmad.attribs.len = PGROUNDUP(entry->size);
						pmad.attribs.type = KN_PMEM_AVAILABLE;
					}
				}
				break;
			case ARCH_MEM_ACPI:
				if (!((entry->base > INIT_CRITICAL_PTOP) ||
						(entry->base + entry->size < INIT_CRITICAL_PBASE))) {
					km_panic("Critical memory area has been occupied by ACPI");
				}
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->size);
				pmad.attribs.type = KN_PMEM_ACPI;
				break;
			case ARCH_MEM_HIBERNATION:
				if (!((entry->base > INIT_CRITICAL_PTOP) ||
						(entry->base + entry->size < INIT_CRITICAL_PBASE))) {
					km_panic("Critical memory area has been occupied by hibernation");
				}
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->size);
				pmad.attribs.type = KN_PMEM_HIBERNATION;
				break;
			case ARCH_MEM_BAD:
				if (!((entry->base > INIT_CRITICAL_PTOP) ||
						(entry->base + entry->size < INIT_CRITICAL_PBASE)))
					km_panic("Detected bad memory area in critical location");
				else
					continue;
			default:
				if (!((entry->base > INIT_CRITICAL_PTOP) ||
						(entry->base + entry->size < INIT_CRITICAL_PBASE)))
					km_panic("Detected reserved memory area in critical location");
				else
					continue;
		}

		hn_push_pmad(&pmad);
	}

	//
	// Detect and set the maximum order for each MADs.
	//
	for (uint8_t i = 0; i < ARRAYLEN(hn_pmad_list); ++i) {
		if (hn_pmad_list[i].attribs.type == KN_PMEM_END)
			break;
		hn_pmad_list[i].attribs.maxord = 0;

		for (uint8_t j = 7; j > 0; j--) {
			if (hn_pmad_list[i].attribs.len % (1 << (j - 1)))
				continue;

			hn_pmad_list[i].attribs.maxord = j;
			break;
		}
	}

	kdprintf("Initialized PMAD list\n");
}

///
/// @brief Initialize paging.
///
static void hn_mm_init_paging() {
	memset((void *)KPDT_VBASE, 0, KPDT_SIZE);
	memset((void *)KPGT_VBASE, 0, KPGT_SIZE);

	//
	// Kernel bottom area
	//
	for (uint32_t vi = PDX(KBOTTOM_VBASE), pi = PDX(KBOTTOM_PBASE), pdi = 0;
		 ((size_t)VADDR(vi, 0, 0)) < KBOTTOM_VTOP; vi++, pi++, pdi++) {
		arch_pte_t *cur = hn_bottom_pgt + (pdi << 10);
		arch_pte_t *pcur = ((arch_pte_t *)KBOTTOMPGT_PBASE) + (pdi << 10);
		hn_kernel_pdt[vi].mask = PDE_P | PDE_RW;
		hn_kernel_pdt[vi].address = PGROUNDDOWN(pcur);

		for (uint32_t j = 0; j < 1024; ++j) {
			cur[j].mask = PTE_P | PTE_RW;
			cur[j].address = PGROUNDDOWN(VADDR(pi, j, 0));
		}
	}

	//
	// Initializable part of critical area
	//
	for (uint32_t vi = PDX(INIT_CRITICAL_VBASE), pi = PDX(INIT_CRITICAL_PBASE), pdi = 0;
		 ((size_t)VADDR(vi, 0, 0)) <= INIT_CRITICAL_VTOP; vi++, pi++, pdi++) {
		arch_pte_t *vcur = hn_kernel_pgt + (pdi << 10);
		arch_pte_t *pcur = ((arch_pte_t *)KPGT_PBASE) + (pdi << 10);
		hn_kernel_pdt[vi].mask = PDE_P | PDE_RW;
		hn_kernel_pdt[vi].address = PGROUNDDOWN(pcur);

		for (uint32_t j = 0; (j < 1024) && ((size_t)VADDR(vi, j, PGOFF_MAX)) <= INIT_CRITICAL_VTOP; ++j) {
			vcur[j].mask = PTE_P | PTE_RW;
			vcur[j].address = PGROUNDDOWN(VADDR(pi, j, 0));
		}
	}

	// Load PDT.
	arch_lpdt(PGROUNDDOWN(KPDT_PBASE));

	kdprintf("Initialized paging\n");
}

///
/// @brief Push a PMAD to the list. The descriptor will be copied.
///
/// @param pmad PMAD to push.
///
FASTCALL_DECL(static void, hn_push_pmad(hn_pmad_t *pmad)) {
	for (uint8_t i = 0; i < ARRAYLEN(hn_pmad_list); ++i)
		if (hn_pmad_list[i].attribs.type == KN_PMEM_END) {
			memcpy(&(hn_pmad_list[i]), pmad, sizeof(hn_pmad_t));
			hn_pmad_list[i + 1].attribs.type = KN_PMEM_END;
			return;
		}
	km_panic("Too many memory map entries");
}

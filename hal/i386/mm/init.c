#include <pbos/km/logger.h>
#include <pbos/km/proc.h>
#include "../mm.h"

hn_kgdt_t hn_kgdt;
hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1] = {
	{ .attribs = { .base = 0, .len = 0, .type = KN_PMEM_END } }
};

size_t hn_tss_storage_num;
arch_tss_t *hn_tss_storage_ptr;
char **hn_tss_stacks;

static void hn_push_pmad(hn_pmad_t *pmad);
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
	{
		pgaddr_t init_madpool_paddr = NULLPG;
		hn_pmad_t *init_madpool_pmad;
		pgaddr_t init_pgtab_paddr = NULLPG;
		hn_pmad_t *init_pgtab_pmad;
		size_t cur_madpool_slot_index = 0;
		hn_madpool_t *last_madpool = NULL;
		{
			bool need_pgtab = false;
			void *init_madpool_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, VMALLOC_NORESERVE);

			if (!(mm_kernel_context->pdt[PDX(init_madpool_vaddr)].mask & PDE_P)) {
				need_pgtab = true;
			}

			PMAD_FOREACH(i) {
				kf_rbtree_init(&i->mad_query_tree, hn_mad_nodecmp, hn_mad_nodefree);
			}

			PMAD_FOREACH(i) {
				if (i->attribs.type != KN_PMEM_AVAILABLE)
					continue;

				pgaddr_t addr_cur = i->attribs.base;

				if (init_madpool_paddr == NULLPG) {
					if (addr_cur < i->attribs.base + i->attribs.len) {
						init_madpool_paddr = addr_cur++;
						init_madpool_pmad = i;
					}
				}

				if (need_pgtab && (init_pgtab_paddr == NULLPG)) {
					if (addr_cur < i->attribs.base + i->attribs.len) {
						init_pgtab_paddr = addr_cur++;
						init_pgtab_pmad = i;
					}
				}

				if ((init_madpool_paddr != NULLPG) &&
					((!need_pgtab) || (init_pgtab_paddr != NULLPG)))
					break;
			}

			if (init_madpool_paddr == NULLPG) {
				km_panic("Cannot allocate memory for initial MAD pool");
			}

			if (need_pgtab) {
				if (init_pgtab_paddr == NULLPG) {
					km_panic("Cannot allocate page table for initial MAD pool");
				}

				{
					pgaddr_t init_pgtab_tmpmap_vaddr = hn_tmpmap(init_pgtab_paddr, 1, PTE_P | PTE_RW);
					memset(UNPGADDR(init_pgtab_tmpmap_vaddr), 0, PAGESIZE);
					hn_tmpunmap(init_pgtab_tmpmap_vaddr);
				}
				mm_kernel_context->pdt[PDX(init_madpool_vaddr)].mask = PDE_P | PDE_RW;
				mm_kernel_context->pdt[PDX(init_madpool_vaddr)].address = init_pgtab_paddr;
			}

			km_result_t result = mm_mmap(mm_kernel_context, init_madpool_vaddr, UNPGADDR(init_madpool_paddr), PAGESIZE, PAGE_READ | PAGE_WRITE, MMAP_NORC);
			assert(KM_SUCCEEDED(result));

			hn_global_mad_pool_list = (hn_madpool_t*)init_madpool_vaddr;

			memset(hn_global_mad_pool_list, 0, PAGESIZE);

			hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].pgaddr = init_madpool_paddr;
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
			++hn_global_mad_pool_list->header.used_num;
			kf_rbtree_insert(&init_madpool_pmad->mad_query_tree, &hn_global_mad_pool_list->descs[cur_madpool_slot_index].node_header);
			++cur_madpool_slot_index;

			if (need_pgtab) {
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].pgaddr = init_madpool_paddr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
				++hn_global_mad_pool_list->header.used_num;
				kf_rbtree_insert(&init_pgtab_pmad->mad_query_tree, &hn_global_mad_pool_list->descs[cur_madpool_slot_index].node_header);
				++cur_madpool_slot_index;
			}
		}

		PMAD_FOREACH(i) {
			if (i->attribs.type != KN_PMEM_AVAILABLE)
				continue;

			for (pgaddr_t j = i->attribs.base; j < i->attribs.base + i->attribs.len; ++j) {
				if (j == init_madpool_paddr)
					continue;
				if (j == init_pgtab_paddr)
					continue;

				if (cur_madpool_slot_index >= PB_ARRAYSIZE(hn_global_mad_pool_list->descs)) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, VMALLOC_NORESERVE);
					bool new_poolpg_need_pgtab = false;

					if (!(mm_kernel_context->pdt[PDX(new_poolpg_vaddr)].mask & PDE_P)) {
						new_poolpg_need_pgtab = true;
					}

					if (new_poolpg_need_pgtab) {
						pgaddr_t new_poolpg_pgtab_paddr = hn_mmctxt_pgtaballoc(mm_kernel_context, PDX(new_poolpg_vaddr));
						if (!ISVALIDPG(new_poolpg_pgtab_paddr)) {
							km_panic("No enough memory for new MAD pool page's corresponding page table");
						}
					}

					km_result_t result = mm_mmap(mm_kernel_context, new_poolpg_vaddr, new_poolpg_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0);
					assert(KM_SUCCEEDED(result));

					cur_madpool_slot_index = 0;

					last_madpool = hn_global_mad_pool_list;
					hn_global_mad_pool_list->header.next = (hn_madpool_t*)new_poolpg_vaddr;
					hn_global_mad_pool_list = (hn_madpool_t*)new_poolpg_vaddr;
					hn_global_mad_pool_list->header.prev = last_madpool;
				}

				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].pgaddr = j;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_FREE;
				++hn_global_mad_pool_list->header.used_num;
				kf_rbtree_insert(&i->mad_query_tree, &hn_global_mad_pool_list->descs[cur_madpool_slot_index].node_header);

				++cur_madpool_slot_index;
			}
		}

		PMAD_FOREACH(i) {
			if (i->attribs.type == KN_PMEM_AVAILABLE)
				continue;

			for (pgaddr_t j = i->attribs.base; j < i->attribs.base + i->attribs.len; ++j) {
				if (j == init_madpool_paddr)
					continue;
				if (j == init_pgtab_paddr)
					continue;

				if (cur_madpool_slot_index >= PB_ARRAYSIZE(hn_global_mad_pool_list->descs)) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PMEM_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_READ | PAGE_WRITE, VMALLOC_NORESERVE);
					bool new_poolpg_need_pgtab = false;

					if (!(mm_kernel_context->pdt[PDX(new_poolpg_vaddr)].mask & PDE_P)) {
						new_poolpg_need_pgtab = true;
					}

					if (new_poolpg_need_pgtab) {
						pgaddr_t new_poolpg_pgtab_paddr = hn_mmctxt_pgtaballoc(mm_kernel_context, PDX(new_poolpg_vaddr));
						if (!ISVALIDPG(new_poolpg_pgtab_paddr)) {
							km_panic("No enough memory for new MAD pool page's corresponding page table");
						}
					}

					km_result_t result = mm_mmap(mm_kernel_context, new_poolpg_vaddr, new_poolpg_paddr, PAGESIZE, PAGE_READ | PAGE_WRITE, 0);
					assert(KM_SUCCEEDED(result));

					cur_madpool_slot_index = 0;

					hn_global_mad_pool_list = (hn_madpool_t*)new_poolpg_vaddr;

					last_madpool = hn_global_mad_pool_list;
					hn_global_mad_pool_list->header.next = (hn_madpool_t*)new_poolpg_vaddr;
					hn_global_mad_pool_list = (hn_madpool_t*)new_poolpg_vaddr;
					hn_global_mad_pool_list->header.prev = last_madpool;
				}

				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].pgaddr = j;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_FREE;
				++hn_global_mad_pool_list->header.used_num;
				kf_rbtree_insert(&i->mad_query_tree, &hn_global_mad_pool_list->descs[cur_madpool_slot_index].node_header);

				++cur_madpool_slot_index;
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
static void hn_push_pmad(hn_pmad_t *pmad) {
	for (uint8_t i = 0; i < PB_ARRAYSIZE(hn_pmad_list); ++i)
		if (hn_pmad_list[i].attribs.type == KN_PMEM_END) {
			memcpy(&(hn_pmad_list[i]), pmad, sizeof(hn_pmad_t));
			hn_pmad_list[i + 1].attribs.type = KN_PMEM_END;
			return;
		}
	km_panic("Too many memory map entries");
}

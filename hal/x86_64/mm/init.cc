#include <hal/x86_64/misc.h>
#include <pbos/km/logger.h>
#include <pbos/km/proc.h>
#include "../mm.hh"

PBOS_EXTERN_C_BEGIN

uint8_t hn_mm_init_stage = HN_MM_INIT_STAGE_INITIAL;

hn_kgdt_t hn_init_kgdt;
hn_pmad_t hn_pmad_list[ARCH_MMAP_MAX + 1];
size_t hn_pmad_number = 0;

size_t hn_tss_storage_num;
arch_tss_t *hn_tss_storage_ptr;

hn_tmpmap_info_t *hn_tmpmap_info_storage;

hn_kgdt_t *hn_gdt_storage_ptr;

const kn_paging_config_t *kn_cur_paging_config;

void *mm_kernel_bottom_mapping_base_vaddr = nullptr;

PBOS_USED char mm_kernel_initial_stack alignas(PAGESIZE)[1024 * 96];

PBOS_USED arch_pte_t mm_kernel_initial_ptt alignas(PAGESIZE)[KINITPTT_SIZE / sizeof(arch_pte_t)];
PBOS_USED arch_pde_t mm_kernel_initial_pdt alignas(PAGESIZE)[KINITPDT_SIZE / sizeof(arch_pde_t)];
PBOS_USED arch_pdpte_t mm_kernel_initial_pdpt alignas(PAGESIZE)[KINITPDPT_SIZE / sizeof(arch_pdpte_t)];
PBOS_USED arch_pml4e_t mm_kernel_initial_pml4t alignas(PAGESIZE)[KINITPML4T_SIZE / sizeof(arch_pml4e_t)];

void *mm_kernel_initial_bottom_ptt_paddr = nullptr;
void *mm_kernel_initial_bottom_pdt_paddr = nullptr;
void *mm_kernel_initial_bottom_pdpt_paddr = nullptr;
void *mm_kernel_initial_ptt_paddr = nullptr;
void *mm_kernel_initial_pdt_paddr = nullptr;
void *mm_kernel_initial_pdpt_paddr = nullptr;
void *mm_kernel_initial_pml4t_paddr = nullptr;

static void hn_push_pmad(hn_pmad_t &&pmad);
static void hn_init_gdt();
static void hn_mm_init_paging();
static void hn_mm_init_pmadlist();
static void hn_mm_init_areas();
static void hn_init_tss();

static hn_tmpmap_info_t hn_kernel_early_tmpmap_info;

void hn_mm_init() {
	mm_kernel_context->pml4t = mm_kernel_initial_pml4t;
	hn_mm_init_stage = HN_MM_INIT_STAGE_INITIAL;

	hn_init_gdt();
	hn_mm_init_pmadlist();
	hn_mm_init_paging();

	hn_mm_init_stage = HN_MM_INIT_STAGE_AREAS_INITIAL;

	hn_mm_init_areas();

	hn_kernel_early_tmpmap_info.tmpmap_base = (void *)KINITTMPMAP_VBASE;
	hn_kernel_early_tmpmap_info.tmpmap_pgtab_base =
		mm_kernel_initial_ptt +
		((PML4X(KINITTMPMAP_VBASE) - PML4X(KBOTTOM_VBASE)) * (512 * 512 * 512) +
			(PDPTX(KINITTMPMAP_VBASE) - PDPTX(KBOTTOM_VBASE)) * (512 * 512) +
			(PDX(KINITTMPMAP_VBASE) - PDX(KBOTTOM_VBASE)) * 512 +
			PTX(KINITTMPMAP_VBASE));

	hn_tmpmap_info_storage = &hn_kernel_early_tmpmap_info;

	hn_mm_init_stage = HN_MM_INIT_STAGE_AREAS_INITED;

	kn_mm_init_kima();

	ps_eu_num = 1;
	if (!(mm_cur_contexts = (mm_context_t **)mm_kmalloc(ps_eu_num * sizeof(mm_context_t *), alignof(mm_context_t *)))) {
		km_panic("Unable to allocate memory context for all CPUs");
	}

	for (ps_euid_t i = 0; i < ps_eu_num; ++i) {
		mm_cur_contexts[i] = mm_kernel_context;
	}

	hn_mm_init_stage = HN_MM_INIT_STAGE_CUR_CONTEXT_LIST_INITED;

	hn_init_tss();

	kd_printf("Initialized memory manager\n");

	void *p = mm_kmalloc(1024, 1);
	kd_printf("p=%p\n", p);
	p = mm_krealloc(p, 768, 1);
	kd_printf("p=%p\n", p);
	p = mm_kmalloc(256, 1);
	kd_printf("p=%p\n", p);

	asm volatile("hlt");
}

static void hn_init_tss() {
	hn_tss_storage_num = 1;
	hn_tss_storage_ptr = (arch_tss_t *)mm_kmalloc(hn_tss_storage_num * sizeof(arch_tss_t), alignof(arch_tss_t));
	if (!hn_tss_storage_ptr) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	memset(hn_tss_storage_ptr, 0, hn_tss_storage_num * sizeof(arch_tss_t));

	hn_gdt_storage_ptr = (hn_kgdt_t *)mm_kmalloc(hn_tss_storage_num * sizeof(hn_kgdt_t), alignof(hn_kgdt_t));
	if (!hn_gdt_storage_ptr) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	for (size_t i = 0; i < hn_tss_storage_num; ++i) {
		memcpy(&hn_gdt_storage_ptr[i], &hn_init_kgdt, sizeof(hn_kgdt_t));
		hn_gdt_storage_ptr[i].tss_desc1 =
			TSSDESC_LOW(((uint32_t)(uintptr_t)(hn_tss_storage_ptr + i)), sizeof(arch_tss_t), GDT_AB_P | GDT_AB_DPL(0) | GDT_SYSTYPE_TSS32, 0);
		hn_gdt_storage_ptr[i].tss_desc2 = TSSDESC_HIGH(((uintptr_t)(hn_tss_storage_ptr + i)));
	}

	arch_lgdt(&hn_gdt_storage_ptr[0], sizeof(hn_kgdt_t) / sizeof(arch_gdt_desc_t));
	arch_ltr(SELECTOR_TSS);

	kd_printf("Initialized TSS\n");
}

static void hn_mm_init_areas() {
	{
		hn_pmad_t *init_madpool_pmad = nullptr;
		hn_pmad_t *init_pgtab_pmad = nullptr;
		size_t cur_madpool_slot_index = 0;
		hn_madpool_t *last_madpool = NULL;
		void *init_madpool_paddr;	 // Physical address of initial MAD pool.
		uint8_t initial_map_result;	 // Result of initial `hn_mm_mmap_early` call.
		bool insert_result;
		{
			void *init_madpool_vaddr = mm_kvmalloc_early(mm_kernel_context, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

			if (!init_madpool_vaddr)
				km_panic("Error allocating virtual memory for initial MAD pool");

			// Find proper initial pages for further initialization.
			PMAD_FOREACH(i) {
				if (i->attribs.type != KN_PMEM_AVAILABLE)
					continue;

				pgsize_t pages_used = 0;

				if (PGROUNDDOWN(i->attribs.len) > 3) {
					// Enough to contain initial page table (PDPTE, PDE, PTE).
					init_pgtab_pmad = i;
					pages_used += 3;
				}

				if (PGROUNDDOWN(i->attribs.len) > (i->attribs.len / PAGESIZE) / PBOS_ARRAYSIZE(hn_madpool_t::descs) + pages_used) {
					// It's enough to contain the whole initial area.
					init_madpool_pmad = i;
				}

				if (init_pgtab_pmad && init_madpool_pmad)
					break;
			}

			if (!init_madpool_pmad)
				km_panic("No sutiable memory area for initializing");

			if (!init_pgtab_pmad)
				km_panic("No sutiable memory area for initializing");

			init_madpool_paddr =
				init_madpool_pmad == init_pgtab_pmad
					? (char *)UNPGADDR(init_pgtab_pmad->attribs.base) + 3 * PAGESIZE
					: UNPGADDR(init_madpool_pmad->attribs.base);

			initial_map_result = hn_mm_mmap_early(
				mm_kernel_context,
				init_madpool_vaddr,
				init_madpool_paddr,
				MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
				UNPGADDR(init_pgtab_pmad->attribs.base),
				(char *)UNPGADDR(init_pgtab_pmad->attribs.base) + 1 * PAGESIZE,
				(char *)UNPGADDR(init_pgtab_pmad->attribs.base) + 2 * PAGESIZE);

			hn_global_mad_pool_list = (hn_madpool_t *)init_madpool_vaddr;

			memset(hn_global_mad_pool_list, 0, PAGESIZE);

			// Mark the initial pages as allocated.
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = PGROUNDDOWN(init_madpool_paddr);
			hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
			++hn_global_mad_pool_list->header.used_num;
			insert_result = init_madpool_pmad->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
			kd_dbgcheck(insert_result, "Error inserting initial PMAD, please report this bug");
			++cur_madpool_slot_index;

			if (initial_map_result & 0b100) {
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = init_pgtab_pmad->attribs.base;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
				++hn_global_mad_pool_list->header.used_num;
				insert_result = init_pgtab_pmad->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
				kd_dbgcheck(insert_result, "Error inserting PDPTE PMAD, please report this bug");
				++cur_madpool_slot_index;
			}

			if (initial_map_result & 0b010) {
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = init_pgtab_pmad->attribs.base + 1;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
				++hn_global_mad_pool_list->header.used_num;
				insert_result = init_pgtab_pmad->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
				kd_dbgcheck(insert_result, "Error inserting PDE PMAD, please report this bug");
				++cur_madpool_slot_index;
			}

			if (initial_map_result & 0b001) {
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = init_pgtab_pmad->attribs.base + 2;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_KERNEL;
				++hn_global_mad_pool_list->header.used_num;
				insert_result = init_pgtab_pmad->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
				kd_dbgcheck(insert_result, "Error inserting PDE PMAD, please report this bug");
				++cur_madpool_slot_index;
			}
		}

		hn_mm_init_stage = HN_MM_INIT_STAGE_INITIAL_AREAS_INITED;

		void *new_poolpg_pdpt_vaddr = nullptr,
			 *new_poolpg_pdt_vaddr = nullptr,
			 *new_poolpg_ptt_vaddr = nullptr;

		PMAD_FOREACH(i) {
			if (i->attribs.type != KN_PMEM_AVAILABLE)
				continue;

			hn_mad_t *prev_free_mad = nullptr;
			for (pgaddr_t j = i->attribs.base; j < i->attribs.base + i->attribs.len; ++j) {
				if (j == PGROUNDDOWN(init_madpool_paddr))
					continue;
				if (initial_map_result & 0b100) {
					if (j == init_pgtab_pmad->attribs.base)
						continue;
				}
				if (initial_map_result & 0b010) {
					if (j == init_pgtab_pmad->attribs.base + 1)
						continue;
				}
				if (initial_map_result & 0b001) {
					if (j == init_pgtab_pmad->attribs.base + 2)
						continue;
				}

				if (cur_madpool_slot_index >= PBOS_ARRAYSIZE(hn_global_mad_pool_list->descs)) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc_early(
						mm_kernel_context,
						PAGESIZE,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

					if ((!new_poolpg_pdpt_vaddr) && (!(new_poolpg_pdpt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDPT for new MAD pool page");
					if ((!new_poolpg_pdt_vaddr) && (!(new_poolpg_pdt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDT for new MAD pool page");
					if ((!new_poolpg_ptt_vaddr) && (!(new_poolpg_ptt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PTT for new MAD pool page");

					uint8_t mmap_result = hn_mm_mmap_early(
						mm_kernel_context,
						new_poolpg_vaddr, new_poolpg_paddr,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
						new_poolpg_pdpt_vaddr, new_poolpg_pdt_vaddr, new_poolpg_ptt_vaddr);

					if (mmap_result & 0b100)
						new_poolpg_pdpt_vaddr = nullptr;
					if (mmap_result & 0b010)
						new_poolpg_pdt_vaddr = nullptr;
					if (mmap_result & 0b001)
						new_poolpg_ptt_vaddr = nullptr;

					cur_madpool_slot_index = 0;

					memset((hn_madpool_t *)new_poolpg_vaddr, 0, PAGESIZE);

					last_madpool = hn_global_mad_pool_list;
					hn_global_mad_pool_list->header.next = (hn_madpool_t *)new_poolpg_vaddr;
					hn_global_mad_pool_list->header.prev = last_madpool;
					hn_global_mad_pool_list = (hn_madpool_t *)new_poolpg_vaddr;
				}

				if (prev_free_mad) {
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = prev_free_mad;
					prev_free_mad->next_free = &hn_global_mad_pool_list->descs[cur_madpool_slot_index];
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				} else {
					i->free_list = &hn_global_mad_pool_list->descs[cur_madpool_slot_index];
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				}
				prev_free_mad = &hn_global_mad_pool_list->descs[cur_madpool_slot_index];

				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = j;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_FREE;
				++hn_global_mad_pool_list->header.used_num;
				insert_result = i->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
				kd_dbgcheck(insert_result, "Error inserting PMAD, please report this bug");

				++cur_madpool_slot_index;
			}
		}

		PMAD_FOREACH(i) {
			if ((i->attribs.type == KN_PMEM_AVAILABLE) ||
				(i->attribs.type == KN_PMEM_CRITICAL))
				continue;

			// Limine occasionally creates a long reserved region,
			// so we chose to skip reserved areas.

			for (pgaddr_t j = i->attribs.base; j < i->attribs.base + i->attribs.len; ++j) {
				if (cur_madpool_slot_index >= PBOS_ARRAYSIZE(hn_global_mad_pool_list->descs)) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc_early(
						mm_kernel_context,
						PAGESIZE,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

					if ((!new_poolpg_pdpt_vaddr) && (!(new_poolpg_pdpt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDPT for new MAD pool page");
					if ((!new_poolpg_pdt_vaddr) && (!(new_poolpg_pdt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDT for new MAD pool page");
					if ((!new_poolpg_ptt_vaddr) && (!(new_poolpg_ptt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PTT for new MAD pool page");

					uint8_t mmap_result = hn_mm_mmap_early(
						mm_kernel_context,
						new_poolpg_vaddr, new_poolpg_paddr,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
						new_poolpg_pdpt_vaddr, new_poolpg_pdt_vaddr, new_poolpg_ptt_vaddr);

					if (mmap_result & 0b100)
						new_poolpg_pdpt_vaddr = nullptr;
					if (mmap_result & 0b010)
						new_poolpg_pdt_vaddr = nullptr;
					if (mmap_result & 0b001)
						new_poolpg_ptt_vaddr = nullptr;

					cur_madpool_slot_index = 0;

					memset((hn_madpool_t *)new_poolpg_vaddr, 0, PAGESIZE);

					last_madpool = hn_global_mad_pool_list;
					hn_global_mad_pool_list->header.next = (hn_madpool_t *)new_poolpg_vaddr;
					hn_global_mad_pool_list->header.prev = last_madpool;
					hn_global_mad_pool_list = (hn_madpool_t *)new_poolpg_vaddr;
				}

				if (j != i->attribs.base) {
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = &hn_global_mad_pool_list->descs[cur_madpool_slot_index - 1];
					hn_global_mad_pool_list->descs[cur_madpool_slot_index - 1].next_free = &hn_global_mad_pool_list->descs[cur_madpool_slot_index];
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				} else {
					i->free_list = &hn_global_mad_pool_list->descs[cur_madpool_slot_index];
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].prev_free = nullptr;
					hn_global_mad_pool_list->descs[cur_madpool_slot_index].next_free = nullptr;
				}
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].flags = MAD_P;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].rb_value = j;
				hn_global_mad_pool_list->descs[cur_madpool_slot_index].type = MAD_ALLOC_FREE;
				++hn_global_mad_pool_list->header.used_num;
				insert_result = i->query_tree.insert(&hn_global_mad_pool_list->descs[cur_madpool_slot_index]);
				kd_dbgcheck(insert_result, "Error inserting PMAD, please report this bug");

				++cur_madpool_slot_index;
			}
		}

		if (new_poolpg_pdpt_vaddr)
			mm_pgfree(new_poolpg_pdpt_vaddr);
		if (new_poolpg_pdt_vaddr)
			mm_pgfree(new_poolpg_pdt_vaddr);
		if (new_poolpg_ptt_vaddr)
			mm_pgfree(new_poolpg_ptt_vaddr);

		/*for (hn_madpool_t *j = hn_global_mad_pool_list; j; j = j->header.next) {
			km_unwrap_result(kn_mm_insert_vpm(mm_kernel_context, j));
		}*/
	}

	/*for (uintptr_t i = KBOTTOM_PBASE; i <= KBOTTOM_PTOP; i += PAGESIZE) {
		mm_refpg((void *)i);
	}*/

	void *new_poolpg_pdpt_vaddr = nullptr,
		 *new_poolpg_pdt_vaddr = nullptr,
		 *new_poolpg_ptt_vaddr = nullptr;
	char *direct_map_base = (char *)DIRECTPHYMEM_VBASE;
	PMAD_FOREACH(i) {
		if (direct_map_base >= (char *)DIRECTPHYMEM_VBASE) {
			i->direct_map_base = nullptr;
			i->direct_map_size = 0;
			continue;
		}
		uint8_t mmap_result;
		size_t size;
		if ((char *)DIRECTPHYMEM_VTOP - UNPGSIZE(i->attribs.len) < direct_map_base) {
			size = (char *)DIRECTPHYMEM_VTOP - direct_map_base;
		} else {
			size = UNPGSIZE(i->attribs.len);
		}

		char *base = (char *)UNPGADDR(i->attribs.base);

		for (size_t j = 0; j < size; j += PAGESIZE) {
			if ((!new_poolpg_pdpt_vaddr) && (!(new_poolpg_pdpt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PDPT for direct physical memory mapping");
			if ((!new_poolpg_pdt_vaddr) && (!(new_poolpg_pdt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PDT for direct physical memory mapping");
			if ((!new_poolpg_ptt_vaddr) && (!(new_poolpg_ptt_vaddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PTT for direct physical memory mapping");

			mmap_result = hn_mm_mmap_early(
				mm_kernel_context,
				direct_map_base + j, base + j,
				MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
				new_poolpg_pdpt_vaddr, new_poolpg_pdt_vaddr, new_poolpg_ptt_vaddr);

			if (mmap_result & 0b100)
				new_poolpg_pdpt_vaddr = nullptr;
			if (mmap_result & 0b010)
				new_poolpg_pdt_vaddr = nullptr;
			if (mmap_result & 0b001)
				new_poolpg_ptt_vaddr = nullptr;
		}
		i->direct_map_base = direct_map_base;
		i->direct_map_size = size;

		direct_map_base += size;
	}

	if (new_poolpg_pdpt_vaddr)
		mm_pgfree(new_poolpg_pdpt_vaddr);
	if (new_poolpg_pdt_vaddr)
		mm_pgfree(new_poolpg_pdt_vaddr);
	if (new_poolpg_ptt_vaddr)
		mm_pgfree(new_poolpg_ptt_vaddr);

	kd_printf("Initialized memory areas\n");
}

///
/// @brief Initialize and load GDT.
///
static void hn_init_gdt() {
	// NULL descriptor.
	hn_init_kgdt.null_desc = GDTDESC(0, 0, 0, 0);

	// Kernel mode descriptors.
	hn_init_kgdt.kcode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_EX | GDT_AB_RW, GDT_FL_L | GDT_FL_GR);
	hn_init_kgdt.kdata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// User mode descriptors.
	hn_init_kgdt.ucode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_DC | GDT_AB_EX, GDT_FL_L | GDT_FL_GR);
	hn_init_kgdt.udata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// 32-bit user mode descriptors.
	// hn_init_kgdt.ucode32_desc =
	// GDTDESC(0, 0, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_DC | GDT_AB_EX, GDT_FL_DB | GDT_FL_GR);
	// hn_init_kgdt.udata32_desc =
	// GDTDESC(0, 0, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// TSS is a stub, we have to reload the GDT later.
	hn_init_kgdt.tss_desc1 =
		GDTDESC(0, 0xfffff, 0, 0);
	hn_init_kgdt.tss_desc2 =
		GDTDESC(0, 0xfffff, 0, 0);

	arch_lgdt(&hn_init_kgdt, sizeof(hn_init_kgdt) / sizeof(arch_gdt_desc_t));

	arch_loadds(SELECTOR_KDATA);
	arch_loades(SELECTOR_KDATA);

	// stub, the initial CPU always has EUID 0
	arch_loadfs(0);

	arch_loadgs(SELECTOR_KDATA);
	arch_loadss(SELECTOR_KDATA);
	arch_loadcs(SELECTOR_KCODE);

	kd_printf("Initialized GDT\n");
}

///
/// @brief Scan and push PMADs.
///
static void hn_mm_init_pmadlist() {
	hn_pmad_list[0].attribs.type = KN_PMEM_END;

	if (hn_limine_memmap_request.response->entry_count > ARCH_MMAP_MAX)
		km_panic("Too many memory maps");

	for (uint16_t i = 0; i < hn_limine_memmap_request.response->entry_count; ++i) {
		limine_memmap_entry *entry = hn_limine_memmap_request.response->entries[i];

		hn_pmad_t pmad = { 0 };

		const uintptr_t entry_max = entry->base + (entry->length - 1);

		switch (entry->type) {
			case LIMINE_MEMMAP_USABLE:
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
				if (entry->base < KBOTTOM_PTOP) {
					if (entry_max >= KBOTTOM_PTOP) {
						pmad.attribs.base = PGROUNDDOWN(entry->base);
						pmad.attribs.len = PGROUNDUP((KBOTTOM_PTOP + 1) - entry->base);
						pmad.attribs.type = KN_PMEM_HARDWARE;
						hn_push_pmad(std::move(pmad));

						if (entry_max != KBOTTOM_PTOP) {
							pmad.attribs.base = PGROUNDDOWN(KBOTTOM_PTOP + 1);
							pmad.attribs.len = PGROUNDUP(entry_max + 1 - KBOTTOM_PTOP);
							pmad.attribs.type = KN_PMEM_AVAILABLE;
							hn_push_pmad(std::move(pmad));
						}
					} else {
						pmad.attribs.base = PGROUNDDOWN(entry->base);
						pmad.attribs.len = PGROUNDUP(entry->length);
						pmad.attribs.type = KN_PMEM_HARDWARE;
						hn_push_pmad(std::move(pmad));
					}
				} else {
					pmad.attribs.base = PGROUNDDOWN(entry->base);
					pmad.attribs.len = PGROUNDUP(entry->length);
					pmad.attribs.type = KN_PMEM_AVAILABLE;
					hn_push_pmad(std::move(pmad));
				}
				break;
			case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			case LIMINE_MEMMAP_ACPI_NVS:
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->length);
				pmad.attribs.type = KN_PMEM_ACPI;

				hn_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_FRAMEBUFFER:
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->length);
				pmad.attribs.type = KN_PMEM_HARDWARE;
				hn_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->length);
				pmad.attribs.type = KN_PMEM_BOOTDATA;
				hn_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_RESERVED:
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->length);
				pmad.attribs.type = KN_PMEM_CRITICAL;
				hn_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_BAD_MEMORY:
				pmad.attribs.base = PGROUNDDOWN(entry->base);
				pmad.attribs.len = PGROUNDUP(entry->length);
				pmad.attribs.type = KN_PMEM_BAD;
				hn_push_pmad(std::move(pmad));
				break;
			default:
				km_panic("Unknown memory map type, perhaps a bootloader bug");
		}
	}

	kd_printf("Initialized PMAD list\n");
}

///
/// @brief Initialize paging.
///
static void hn_mm_init_paging() {
	memset(mm_kernel_initial_ptt, 0, sizeof(mm_kernel_initial_ptt));
	memset(mm_kernel_initial_pdt, 0, sizeof(mm_kernel_initial_pdt));
	memset(mm_kernel_initial_pdpt, 0, sizeof(mm_kernel_initial_pdpt));
	memset(mm_kernel_initial_pml4t, 0, sizeof(mm_kernel_initial_pml4t));

	mm_kernel_initial_ptt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)mm_kernel_initial_ptt) - hn_limine_executable_address_request.response->virtual_base) + hn_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pdt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)mm_kernel_initial_pdt) - hn_limine_executable_address_request.response->virtual_base) + hn_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pdpt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)mm_kernel_initial_pdpt) - hn_limine_executable_address_request.response->virtual_base) + hn_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pml4t_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)mm_kernel_initial_pml4t) - hn_limine_executable_address_request.response->virtual_base) + hn_limine_executable_address_request.response->physical_base);

	// Map the high 2GB kernel space.
	static_assert(PML4X(CRITICAL_VTOP) - PML4X(KBOTTOM_VBASE) == 0);
	// Fill the PML4 Table.
	{
		const uint32_t pml4x = PML4X(KBOTTOM_VBASE);
		char *const vaddr_pml4 = (char *)KVADDR(pml4x, 0, 0, 0, 0);

		arch_pml4e_t *vpml4e = &mm_kernel_initial_pml4t[pml4x];

		vpml4e->mask = PML4E_P | PML4E_RW | PML4E_U;
		vpml4e->address = PGROUNDDOWN(
			((arch_pdpte_t *)mm_kernel_initial_pdpt_paddr) +
			((PML4X(vaddr_pml4) - PML4X(KBOTTOM_VBASE)) * 512));

		kd_assert(UNPGADDR(vpml4e->address) < ((arch_pdpte_t *)mm_kernel_initial_pdpt_paddr) + PBOS_ARRAYSIZE(mm_kernel_initial_ptt));

		// Fill the PDP Table.
		for (uint32_t pdptx = (pml4x == PML4X(KBOTTOM_VBASE) ? PDPTX(KBOTTOM_VBASE) : 0); pdptx < PDPTX_MAX + 1; ++pdptx) {
			char *const vaddr_pdpt = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);
			arch_pdpte_t *vpdpte = &mm_kernel_initial_pdpt[(PML4X(vaddr_pdpt) - PML4X(KBOTTOM_VBASE)) * 512 + pdptx];

			vpdpte->mask = PDPTE_P | PDPTE_RW | PDPTE_U;
			vpdpte->address = PGROUNDDOWN(
				((arch_pde_t *)mm_kernel_initial_pdt_paddr) +
				((PML4X(vaddr_pdpt) - PML4X(KBOTTOM_VBASE)) * 512 * 512 +
					(PDPTX(vaddr_pdpt) - PDPTX(KBOTTOM_VBASE)) * 512));

			// Fill the Page Directory Table.
			for (uint32_t pdx = (pml4x == PML4X(KBOTTOM_VBASE) &&
										 pdptx == PDPTX(KBOTTOM_VBASE)
									 ? PDX(KBOTTOM_VBASE)
									 : 0);
				pdx < PDX_MAX + 1; ++pdx) {
				char *const vaddr_pd = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);
				arch_pde_t *vpde = &mm_kernel_initial_pdt[(
					(PML4X(vaddr_pd) - PML4X(KBOTTOM_VBASE)) * 512 * 512 +
					(PDPTX(vaddr_pd) - PDPTX(KBOTTOM_VBASE)) * 512 +
					pdx)];

				vpde->mask = PDE_P | PDE_RW | PDE_U;
				vpde->address = PGROUNDDOWN(
					((arch_pte_t *)mm_kernel_initial_ptt_paddr) +
					(((PML4X(vaddr_pd) - PML4X(KBOTTOM_VBASE)) * 512 * 512 * 512 +
						(PDPTX(vaddr_pd) - PDPTX(KBOTTOM_VBASE)) * 512 * 512 +
						(PDX(vaddr_pd) - PDX(KBOTTOM_VBASE)) * 512)));

				if ((((uintptr_t)vaddr_pd) > KBOTTOM_VTOP) &&
					(((uintptr_t)KVADDR(pml4x, pdptx, pdx, PTX_MAX, PGOFF_MAX)) < CRITICAL_VBASE))
					continue;

				// Fill the Page Table Table.
				for (uint32_t ptx = (pml4x == PML4X(KBOTTOM_VBASE) &&
											 pdptx == PDPTX(KBOTTOM_VBASE) &&
											 pdx == PDX(KBOTTOM_VBASE)
										 ? PTX(KBOTTOM_VBASE)
										 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const vaddr_pt = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);
					if ((((uintptr_t)vaddr_pt) > KBOTTOM_VTOP) &&
						(((uintptr_t)vaddr_pt) < CRITICAL_VBASE))
						continue;
					if (((uintptr_t)vaddr_pt) > INIT_CRITICAL_VTOP)
						goto fill_end;
					arch_pte_t *vpte = &mm_kernel_initial_ptt[(
																  ((PML4X(vaddr_pt) - PML4X(KBOTTOM_VBASE)) * 512 * 512 * 512 +
																	  (PDPTX(vaddr_pt) - PDPTX(KBOTTOM_VBASE)) * 512 * 512 +
																	  (PDX(vaddr_pt) - PDX(KBOTTOM_VBASE)) * 512)) +
															  ptx];

					if ((((uintptr_t)vaddr_pt) > KBOTTOM_VTOP) &&
						(((uintptr_t)vaddr_pt) < CRITICAL_VBASE)) {
						vpte->address = PGROUNDDOWN(vaddr_pt - KBOTTOM_VBASE);
					} else {
						vpte->address = PGROUNDDOWN(
							(((char *)hn_limine_executable_address_request.response->physical_base)) +
							(((uintptr_t)vaddr_pt) - CRITICAL_VBASE));
					}
					vpte->mask = PTE_P | PTE_RW;
				}
			}
		}
	}

	mm_kernel_bottom_mapping_base_vaddr = (void *)KBOTTOM_VBASE;

fill_end:
	// Load PDT.
	arch_lpgtab(PGROUNDDOWN(mm_kernel_initial_pml4t_paddr));

	kn_cur_paging_config = &KN_PAGING_CONFIG_48BIT;

	kd_printf("Initialized paging\n");
}

///
/// @brief Push a PMAD to the list. The descriptor will be copied.
///
/// @param pmad PMAD to push.
///
static void hn_push_pmad(hn_pmad_t &&pmad) {
	if (hn_pmad_number + 1 >= PBOS_ARRAYSIZE(hn_pmad_list))
		km_panic("Too many memory map entries");
	hn_pmad_list[hn_pmad_number++] = std::move(pmad);
	hn_pmad_list[hn_pmad_number].attribs.type = KN_PMEM_END;
}

PBOS_EXTERN_C_END

#include <hal/x86_64/misc.h>
#include <pbos/kd/logger.h>
#include <pbos/ki/ds/blfb.h>
#include <pbos/ki/kf/misc.h>
#include <pbos/ps/proc.h>
#include <hal/x86_64/ds/blfb.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/ki/acpi/rsdt.hh>
#include <pbos/ki/mm/pgalloc.hh>
#include "../mm.hh"

PBOS_EXTERN_C_BEGIN

hali_kgdt_t hali_init_kgdt;

void *mm_kernel_bottom_mapping_base_vaddr = nullptr;

PBOS_USED char mm_kernel_initial_stack alignas(PAGESIZE)[1024 * 48];

PBOS_USED arch_pte_t mm_kernel_initial_ptt alignas(PAGESIZE)[KINITPTT_SIZE / sizeof(arch_pte_t)];
PBOS_USED arch_pde_t mm_kernel_initial_pdt alignas(PAGESIZE)[KINITPDT_SIZE / sizeof(arch_pde_t)];
PBOS_USED arch_pdpte_t mm_kernel_initial_pdpt alignas(PAGESIZE)[KINITPDPT_SIZE / sizeof(arch_pdpte_t)];
PBOS_USED arch_pml4te_t mm_kernel_initial_pml4t alignas(PAGESIZE)[KINITPML4T_SIZE / sizeof(arch_pml4te_t)];

PBOS_USED char mm_kernel_init_tmpmap_area alignas(PAGESIZE)[KINITTMPMAP_SIZE];

void *mm_kernel_initial_bottom_ptt_paddr = nullptr;
void *mm_kernel_initial_bottom_pdt_paddr = nullptr;
void *mm_kernel_initial_bottom_pdpt_paddr = nullptr;
void *mm_kernel_initial_ptt_paddr = nullptr;
void *mm_kernel_initial_pdt_paddr = nullptr;
void *mm_kernel_initial_pdpt_paddr = nullptr;
void *mm_kernel_initial_pml4t_paddr = nullptr;

PBOS_NO_ASAN static void hali_push_pmad(ki_pmad_t &&pmad);
PBOS_NO_ASAN static void hali_init_gdt();
PBOS_NO_ASAN static void hali_mm_init_paging();
PBOS_NO_ASAN static void hali_mm_init_pmadlist();
PBOS_NO_ASAN static void hali_mm_init_areas();

static hali_tmpmap_info_t hali_kernel_early_tmpmap_info;

PBOS_NO_ASAN void kh_mm_init() {
	mm_kernel_context->page_table = mm_kernel_initial_pml4t;

	hali_init_gdt();
	hali_mm_init_pmadlist();

	// Collect INITCAR's physical address.
	if ((!hali_limine_module_request.response) || (hali_limine_module_request.response->module_count != 1))
		km_panic("Invalid module count, the module count passing to the kernel should be 1 (the initcar only)");
	limine_file *initcar_file = hali_limine_module_request.response->modules[0];
	ki_initcar_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)initcar_file->address) - hali_limine_hhdm_request.response->offset));
	ki_initcar_file_size = initcar_file->size;

	// Collect ACPI RSDP's physical address.
	ki_acpi_rsdp_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)hali_limine_rsdp_request.response->address) - hali_limine_hhdm_request.response->offset));

	// Collect framebuffer information.
	if (hali_limine_framebuffer_request.response) {
		const size_t cnt = hali_limine_framebuffer_request.response->framebuffer_count;
		for (size_t i = 0; i < cnt; ++i) {
			auto fb = hali_limine_framebuffer_request.response->framebuffers[i];

			ds_pixel_format_t pf = DS_PIXEL_FORMAT_UNKNOWN;

			for (size_t j = 0; j < ki_blfb_pixel_format_case_num; ++j) {
				if ((ki_blfb_pixel_format_cases[j].red.shift == fb->red_mask_shift) &&
					(ki_blfb_pixel_format_cases[j].red.size == fb->red_mask_size) &&
					(ki_blfb_pixel_format_cases[j].green.shift == fb->green_mask_shift) &&
					(ki_blfb_pixel_format_cases[j].green.size == fb->green_mask_size) &&
					(ki_blfb_pixel_format_cases[j].blue.shift == fb->blue_mask_shift) &&
					(ki_blfb_pixel_format_cases[j].blue.size == fb->blue_mask_size)) {
					pf = ki_blfb_pixel_format_cases[j].format;

					if (fb->pitch > UINT32_MAX) {
						kd_printf("Stride out-of-range by BLFB #%lu: %lu\n", static_cast<uint64_t>(i), static_cast<uint64_t>(j));
						break;
					}

					ki_blfb_desc_t desc = {
						.paddr = reinterpret_cast<void *>(static_cast<char *>(fb->address) - static_cast<char *>(mm_kernel_bottom_mapping_base_vaddr)),
						.mapped_vaddr = nullptr,
						.pixel_format = pf,
						.width = fb->width,
						.height = fb->height,
						.stride = static_cast<uint32_t>(fb->pitch)
					};

					hali_push_blfb_desc(desc);
					break;
				}
			}

			if (pf == DS_PIXEL_FORMAT_UNKNOWN)
				kd_printf("BLFB #%lu has unknown pixel format, ignored\n", static_cast<uint64_t>(i));
		}
	}

	hali_mm_init_paging();

	kh_mad_pool_descs_off = kfxx::ceil_align_to((uintptr_t)sizeof(ki_madpool_header_t), alignof(ki_mad_t));
	kh_mad_pool_descs_num_per_page = (mm_get_page_size() - kh_mad_pool_descs_off) / sizeof(ki_mad_t);

	hali_mm_init_areas();

	hali_kernel_early_tmpmap_info.tmpmap_base = (void *)KINITTMPMAP_VBASE;
	hali_kernel_early_tmpmap_info.tmpmap_pgtab_base =
		mm_kernel_initial_ptt +
		((PML4X(KINITTMPMAP_VBASE) - PML4X(KERNEL_VBASE)) * (512 * 512 * 512) +
			(PDPTX(KINITTMPMAP_VBASE) - PDPTX(KERNEL_VBASE)) * (512 * 512) +
			(PDX(KINITTMPMAP_VBASE) - PDX(KERNEL_VBASE)) * 512 +
			PTX(KINITTMPMAP_VBASE));

	hali_tmpmap_storage_ptr = &hali_kernel_early_tmpmap_info;
}

PBOS_NO_ASAN static void hali_mm_init_areas() {
	{
		ki_pmad_t *init_madpool_pmad = nullptr;
		ki_pmad_t *init_pgtab_pmad = nullptr;
		size_t cur_madpool_slot_index = 0;
		ki_madpool_t *last_madpool = NULL;
		void *init_madpool_paddr;	 // Physical address of initial MAD pool.
		uint8_t initial_map_result;	 // Result of initial `hali_mm_mmap_early` call.
		bool insert_result;
		{
			void *init_madpool_vaddr = mm_kvmalloc_early(mm_kernel_context, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

			if (!init_madpool_vaddr)
				km_panic("Error allocating virtual memory for initial MAD pool");

			// Find proper initial pages for further initialization.
			KI_PMAD_FOREACH(i) {
				if (i->type != MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)
					continue;

				pgsize_t pages_used = 0;

				if (PGROUNDDOWN(i->len) > 3) {
					// Enough to contain initial page table (PDPTE, PDE, PTE).
					init_pgtab_pmad = i;
					pages_used += 3;
				}

				if (PGROUNDDOWN(i->len) > (i->len / PAGESIZE) / kh_mad_pool_descs_num_per_page + pages_used) {
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
					? (char *)init_pgtab_pmad->rb_value + 3 * PAGESIZE
					: init_madpool_pmad->rb_value;

			initial_map_result = hali_mm_mmap_early(
				mm_kernel_context,
				init_madpool_vaddr,
				init_madpool_paddr,
				MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
				init_pgtab_pmad->rb_value,
				(char *)init_pgtab_pmad->rb_value + 1 * PAGESIZE,
				(char *)init_pgtab_pmad->rb_value + 2 * PAGESIZE);

			ki_global_mad_pool_list = (ki_madpool_t *)init_madpool_vaddr;

			ki_raw_memset(ki_global_mad_pool_list, 0, PAGESIZE);

			ki_mad_t *descs = (ki_mad_t *)(((uintptr_t)ki_global_mad_pool_list) + kh_mad_pool_descs_off);

			// Mark the initial pages as allocated.
			kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
			descs[cur_madpool_slot_index].next_free = nullptr;
			descs[cur_madpool_slot_index].prev_free = nullptr;
			descs[cur_madpool_slot_index].rb_value = init_madpool_paddr;
			++descs[cur_madpool_slot_index].pin_count;
			++ki_global_mad_pool_list->header.used_num;
			init_madpool_pmad->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);
			++cur_madpool_slot_index;

			++init_madpool_pmad->used_count;

			if (initial_map_result & 0b100) {
				kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
				descs[cur_madpool_slot_index].next_free = nullptr;
				descs[cur_madpool_slot_index].prev_free = nullptr;
				descs[cur_madpool_slot_index].rb_value = init_pgtab_pmad->rb_value;
				++descs[cur_madpool_slot_index].pin_count;
				++ki_global_mad_pool_list->header.used_num;
				init_pgtab_pmad->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);
				++cur_madpool_slot_index;

				++init_pgtab_pmad->used_count;
			}

			if (initial_map_result & 0b010) {
				kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
				descs[cur_madpool_slot_index].next_free = nullptr;
				descs[cur_madpool_slot_index].prev_free = nullptr;
				descs[cur_madpool_slot_index].rb_value = (char *)init_pgtab_pmad->rb_value + PAGESIZE * 1;
				++descs[cur_madpool_slot_index].pin_count;
				++ki_global_mad_pool_list->header.used_num;
				init_pgtab_pmad->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);
				++cur_madpool_slot_index;

				++init_pgtab_pmad->used_count;
			}

			if (initial_map_result & 0b001) {
				kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
				descs[cur_madpool_slot_index].next_free = nullptr;
				descs[cur_madpool_slot_index].prev_free = nullptr;
				descs[cur_madpool_slot_index].rb_value = (char *)init_pgtab_pmad->rb_value + PAGESIZE * 2;
				++descs[cur_madpool_slot_index].pin_count;
				++ki_global_mad_pool_list->header.used_num;
				init_pgtab_pmad->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);
				++cur_madpool_slot_index;

				++init_pgtab_pmad->used_count;
			}
		}

		void *new_poolpg_pdpt_paddr = nullptr,
			 *new_poolpg_pdt_paddr = nullptr,
			 *new_poolpg_ptt_paddr = nullptr;

		KI_PMAD_FOREACH(i) {
			if (i->type != MM_PHYSICAL_MEMORY_TYPE_AVAILABLE)
				continue;

			ki_mad_t *prev_free_mad = nullptr;
			for (char *j = (char *)i->rb_value; j < (char *)i->rb_value + i->len; j += PAGESIZE) {
				if (j == init_madpool_paddr)
					continue;
				if (initial_map_result & 0b100) {
					if (j == init_pgtab_pmad->rb_value)
						continue;
				}
				if (initial_map_result & 0b010) {
					if (j == (char *)init_pgtab_pmad->rb_value + 1 * PAGESIZE)
						continue;
				}
				if (initial_map_result & 0b001) {
					if (j == (char *)init_pgtab_pmad->rb_value + 2 * PAGESIZE)
						continue;
				}

				if (cur_madpool_slot_index >= kh_mad_pool_descs_num_per_page) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc_early(
						mm_kernel_context,
						PAGESIZE,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

					if ((!new_poolpg_pdpt_paddr) && (!(new_poolpg_pdpt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDPT for new MAD pool page");
					if ((!new_poolpg_pdt_paddr) && (!(new_poolpg_pdt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDT for new MAD pool page");
					if ((!new_poolpg_ptt_paddr) && (!(new_poolpg_ptt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PTT for new MAD pool page");

					uint8_t mmap_result = hali_mm_mmap_early(
						mm_kernel_context,
						new_poolpg_vaddr, new_poolpg_paddr,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
						new_poolpg_pdpt_paddr, new_poolpg_pdt_paddr, new_poolpg_ptt_paddr);

					if (mmap_result & 0b100) {
						new_poolpg_pdpt_paddr = nullptr;
					}
					if (mmap_result & 0b010) {
						new_poolpg_pdt_paddr = nullptr;
					}
					if (mmap_result & 0b001) {
						new_poolpg_ptt_paddr = nullptr;
					}

					cur_madpool_slot_index = 0;

					ki_raw_memset((ki_madpool_t *)new_poolpg_vaddr, 0, PAGESIZE);

					last_madpool = ki_global_mad_pool_list;
					((ki_madpool_t *)new_poolpg_vaddr)->header.next = ki_global_mad_pool_list;
					ki_global_mad_pool_list->header.prev = ((ki_madpool_t *)new_poolpg_vaddr);
					ki_global_mad_pool_list = (ki_madpool_t *)new_poolpg_vaddr;
				}

				ki_mad_t *descs = (ki_mad_t *)(((uintptr_t)ki_global_mad_pool_list) + kh_mad_pool_descs_off);

				kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
				if (prev_free_mad) {
					descs[cur_madpool_slot_index].prev_free = prev_free_mad;
					prev_free_mad->next_free = &descs[cur_madpool_slot_index];
					descs[cur_madpool_slot_index].next_free = nullptr;
				} else {
					i->free_list = &descs[cur_madpool_slot_index];
					descs[cur_madpool_slot_index].prev_free = nullptr;
					descs[cur_madpool_slot_index].next_free = nullptr;
				}
				prev_free_mad = &descs[cur_madpool_slot_index];

				descs[cur_madpool_slot_index].rb_value = j;
				++ki_global_mad_pool_list->header.used_num;
				i->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);

				++cur_madpool_slot_index;
			}
		}

		KI_PMAD_FOREACH(i) {
			if ((i->type == MM_PHYSICAL_MEMORY_TYPE_AVAILABLE) ||
				(i->type == MM_PHYSICAL_MEMORY_TYPE_CRITICAL))
				continue;

			// Limine occasionally creates a long reserved region,
			// so we chose to skip reserved areas.

			for (char *j = (char *)i->rb_value; j < (char *)i->rb_value + i->len; j += PAGESIZE) {
				if (cur_madpool_slot_index >= kh_mad_pool_descs_num_per_page) {
					void *new_poolpg_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);
					if (!new_poolpg_paddr)
						km_panic("No enough physical memory for new MAD pool page");
					void *new_poolpg_vaddr = mm_kvmalloc_early(
						mm_kernel_context,
						PAGESIZE,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE);

					if (!new_poolpg_vaddr)
						km_panic("Error allocating virtual memory for new MAD pool page");

					if ((!new_poolpg_pdpt_paddr) && (!(new_poolpg_pdpt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDPT for new MAD pool page");
					if ((!new_poolpg_pdt_paddr) && (!(new_poolpg_pdt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PDT for new MAD pool page");
					if ((!new_poolpg_ptt_paddr) && (!(new_poolpg_ptt_paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
						km_panic("Error allocating PTT for new MAD pool page");

					uint8_t mmap_result = hali_mm_mmap_early(
						mm_kernel_context,
						new_poolpg_vaddr, new_poolpg_paddr,
						MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
						new_poolpg_pdpt_paddr, new_poolpg_pdt_paddr, new_poolpg_ptt_paddr);

					if (mmap_result & 0b100) {
						new_poolpg_pdpt_paddr = nullptr;
					}
					if (mmap_result & 0b010) {
						new_poolpg_pdt_paddr = nullptr;
					}
					if (mmap_result & 0b001) {
						new_poolpg_ptt_paddr = nullptr;
					}

					cur_madpool_slot_index = 0;

					ki_raw_memset((ki_madpool_t *)new_poolpg_vaddr, 0, PAGESIZE);

					last_madpool = ki_global_mad_pool_list;
					((ki_madpool_t *)new_poolpg_vaddr)->header.next = ki_global_mad_pool_list;
					ki_global_mad_pool_list->header.prev = ((ki_madpool_t *)new_poolpg_vaddr);
					ki_global_mad_pool_list = (ki_madpool_t *)new_poolpg_vaddr;
				}
				ki_mad_t *descs = (ki_mad_t *)(((uintptr_t)ki_global_mad_pool_list) + kh_mad_pool_descs_off);
				kfxx::construct_at<ki_mad_t>(&descs[cur_madpool_slot_index]);
				if (j != i->rb_value) {
					descs[cur_madpool_slot_index].prev_free = &descs[cur_madpool_slot_index - 1];
					descs[cur_madpool_slot_index - 1].next_free = &descs[cur_madpool_slot_index];
					descs[cur_madpool_slot_index].next_free = nullptr;
				} else {
					i->free_list = &descs[cur_madpool_slot_index];
					descs[cur_madpool_slot_index].prev_free = nullptr;
					descs[cur_madpool_slot_index].next_free = nullptr;
				}
				descs[cur_madpool_slot_index].rb_value = j;
				++ki_global_mad_pool_list->header.used_num;
				i->query_tree.insert_unwrap(&descs[cur_madpool_slot_index]);

				++cur_madpool_slot_index;
			}
		}

		if (new_poolpg_pdpt_paddr)
			mm_unpin_page(new_poolpg_pdpt_paddr);
		if (new_poolpg_pdt_paddr)
			mm_unpin_page(new_poolpg_pdt_paddr);
		if (new_poolpg_ptt_paddr)
			mm_unpin_page(new_poolpg_ptt_paddr);
	}

	void *new_poolpg_pdpt_page = nullptr,
		 *new_poolpg_pdt_page = nullptr,
		 *new_poolpg_ptt_page = nullptr;
	char *direct_map_base = (char *)DIRECTPHYMEM_VBASE;
	KI_PMAD_FOREACH(i) {
		if ((i->type == MM_PHYSICAL_MEMORY_TYPE_CRITICAL) || (direct_map_base >= (char *)DIRECTPHYMEM_VTOP)) {
			i->direct_map_base = nullptr;
			i->direct_map_size = 0;
			continue;
		}
		uint8_t mmap_result;
		size_t size;
		if ((char *)DIRECTPHYMEM_VTOP - i->len < direct_map_base) {
			size = (char *)DIRECTPHYMEM_VTOP - direct_map_base;
		} else {
			size = i->len;
		}

		char *base = (char *)i->rb_value;

		for (size_t j = 0; j < size; j += PAGESIZE) {
			if ((!new_poolpg_pdpt_page) && (!(new_poolpg_pdpt_page = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PDPT for direct physical memory mapping");
			if ((!new_poolpg_pdt_page) && (!(new_poolpg_pdt_page = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PDT for direct physical memory mapping");
			if ((!new_poolpg_ptt_page) && (!(new_poolpg_ptt_page = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE))))
				km_panic("Error allocating PTT for direct physical memory mapping");

			mmap_result = hali_mm_mmap_early(
				mm_kernel_context,
				direct_map_base + j, base + j,
				MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE,
				new_poolpg_pdpt_page, new_poolpg_pdt_page, new_poolpg_ptt_page);

			if (mmap_result & 0b100)
				new_poolpg_pdpt_page = nullptr;
			if (mmap_result & 0b010)
				new_poolpg_pdt_page = nullptr;
			if (mmap_result & 0b001)
				new_poolpg_ptt_page = nullptr;
		}
		i->direct_map_base = direct_map_base;
		i->direct_map_size = size;

		direct_map_base += size;
	}

	if (new_poolpg_pdpt_page)
		mm_unpin_page(new_poolpg_pdpt_page);
	if (new_poolpg_pdt_page)
		mm_unpin_page(new_poolpg_pdt_page);
	if (new_poolpg_ptt_page)
		mm_unpin_page(new_poolpg_ptt_page);
}

///
/// @brief Initialize and load GDT.
///
PBOS_NO_ASAN static void hali_init_gdt() {
	// NULL descriptor.
	hali_init_kgdt.null_desc = GDTDESC(0, 0, 0, 0);

	// Kernel mode descriptors.
	hali_init_kgdt.kcode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_EX | GDT_AB_RW, GDT_FL_L | GDT_FL_GR);
	hali_init_kgdt.kdata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(0) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// User mode descriptors.
	hali_init_kgdt.ucode_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_EX | GDT_AB_RW, GDT_FL_L | GDT_FL_GR);
	hali_init_kgdt.udata_desc =
		GDTDESC(0, 0xfffff, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// 32-bit user mode descriptors.
	// hali_init_kgdt.ucode32_desc =
	// GDTDESC(0, 0, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_DC | GDT_AB_EX, GDT_FL_DB | GDT_FL_GR);
	// hali_init_kgdt.udata32_desc =
	// GDTDESC(0, 0, GDT_AB_P | GDT_AB_DPL(3) | GDT_AB_S | GDT_AB_RW, GDT_FL_DB | GDT_FL_GR);

	// TSS is a stub, we have to reload the GDT later.
	hali_init_kgdt.tss_desc1 =
		GDTDESC(0, 0xfffff, 0, 0);
	hali_init_kgdt.tss_desc2 =
		GDTDESC(0, 0xfffff, 0, 0);

	arch_lgdt(&hali_init_kgdt, sizeof(hali_init_kgdt) / sizeof(arch_gdt_desc_t));

	arch_loadds(SELECTOR_KDATA);
	arch_loades(SELECTOR_KDATA);

	// stub, the initial CPU always has CPUID 0
	arch_loadfs(0);

	arch_loadgs(SELECTOR_KDATA);
	arch_loadss(SELECTOR_KDATA);
	arch_loadcs(SELECTOR_KCODE);
}

///
/// @brief Scan and push PMADs.
///
PBOS_NO_ASAN static void hali_mm_init_pmadlist() {
	if (hali_limine_memmap_request.response->entry_count > KI_INITIAL_MM_AREA_STORAGE_NUM)
		km_panic("Too many initial memory maps");

	for (uint16_t i = 0; i < hali_limine_memmap_request.response->entry_count; ++i) {
		limine_memmap_entry *entry = hali_limine_memmap_request.response->entries[i];

		ki_pmad_t pmad = {};

		pmad.is_initial_pmad = true;

		const uintptr_t entry_max = entry->base + (entry->length - 1);

		switch (entry->type) {
			case LIMINE_MEMMAP_USABLE:
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
				if (!entry->base) {
					pmad.rb_value = (void *)0;
					pmad.len = PAGESIZE;
					pmad.type = MM_PHYSICAL_MEMORY_TYPE_HARDWARE;
					hali_push_pmad(std::move(pmad));
					pmad = {};
					pmad.rb_value = (void *)PAGESIZE;
					pmad.len = entry->length - PAGESIZE;
					pmad.type = MM_PHYSICAL_MEMORY_TYPE_AVAILABLE;
					hali_push_pmad(std::move(pmad));
				} else {
					pmad.rb_value = (void *)entry->base;
					pmad.len = entry->length;
					pmad.type = MM_PHYSICAL_MEMORY_TYPE_AVAILABLE;
					hali_push_pmad(std::move(pmad));
				}
				break;
			case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			case LIMINE_MEMMAP_ACPI_NVS:
				pmad.rb_value = (void *)entry->base;
				pmad.len = entry->length;
				pmad.type = MM_PHYSICAL_MEMORY_TYPE_ACPI;

				hali_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_FRAMEBUFFER:
				pmad.rb_value = (void *)entry->base;
				pmad.len = entry->length;
				pmad.type = MM_PHYSICAL_MEMORY_TYPE_HARDWARE;
				hali_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
				pmad.rb_value = (void *)entry->base;
				pmad.len = entry->length;
				pmad.type = MM_PHYSICAL_MEMORY_TYPE_BOOTDATA;
				hali_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_RESERVED:
				pmad.rb_value = (void *)entry->base;
				pmad.len = entry->length;
				pmad.type = MM_PHYSICAL_MEMORY_TYPE_CRITICAL;
				hali_push_pmad(std::move(pmad));
				break;
			case LIMINE_MEMMAP_BAD_MEMORY:
				pmad.rb_value = (void *)entry->base;
				pmad.len = entry->length;
				pmad.type = MM_PHYSICAL_MEMORY_TYPE_BAD;
				hali_push_pmad(std::move(pmad));
				break;
			default:
				km_panic("Unknown memory map type, perhaps a bootloader bug");
		}
	}
}

///
/// @brief Initialize paging. This also collects physical address of ACPI RSDP.
///
PBOS_NO_ASAN static void hali_mm_init_paging() {
	ki_raw_memset(mm_kernel_initial_ptt, 0, sizeof(mm_kernel_initial_ptt));
	ki_raw_memset(mm_kernel_initial_pdt, 0, sizeof(mm_kernel_initial_pdt));
	ki_raw_memset(mm_kernel_initial_pdpt, 0, sizeof(mm_kernel_initial_pdpt));
	ki_raw_memset(mm_kernel_initial_pml4t, 0, sizeof(mm_kernel_initial_pml4t));

	mm_kernel_initial_ptt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)&mm_kernel_initial_ptt[0]) - hali_limine_executable_address_request.response->virtual_base) + hali_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pdt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)&mm_kernel_initial_pdt[0]) - hali_limine_executable_address_request.response->virtual_base) + hali_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pdpt_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)&mm_kernel_initial_pdpt[0]) - hali_limine_executable_address_request.response->virtual_base) + hali_limine_executable_address_request.response->physical_base);
	mm_kernel_initial_pml4t_paddr = (void *)((~0xffff000000000000) & (uint64_t)(((char *)&mm_kernel_initial_pml4t[0]) - hali_limine_executable_address_request.response->virtual_base) + hali_limine_executable_address_request.response->physical_base);

	// Map the high 2GB kernel space.
	// Fill the PML4 Table.
	{
		const uint32_t pml4x = PML4X(KERNEL_VBASE);
		char *const vaddr_pml4 = (char *)KVADDR(pml4x, 0, 0, 0, 0);

		arch_pml4te_t *vpml4te = &mm_kernel_initial_pml4t[pml4x];

		*vpml4te =
			ARCH_PML4TE_WITH_XD(
				ARCH_PML4TE_WITH_ADDR(
					ARCH_PML4TE_WITH_MASKS(0, PML4E_P | PML4E_RW | PML4E_U),
					PGROUNDDOWN(
						((arch_pdpte_t *)mm_kernel_initial_pdpt_paddr) +
						((PML4X(vaddr_pml4) - PML4X(KERNEL_VBASE)) * 512))),
				false);

		kd_assert(UNPGADDR(ARCH_PML4TE_ADDR(*vpml4te)) < ((arch_pdpte_t *)mm_kernel_initial_pdpt_paddr) + PBOS_ARRAYSIZE(mm_kernel_initial_ptt));

		// Fill the PDP Table.

#define PDPT_MATCH_INITIAL_COND (pml4x == PML4X(KERNEL_VBASE))
		for (uint32_t pdptx = (PDPT_MATCH_INITIAL_COND ? PDPTX(KERNEL_VBASE) : 0); pdptx < PDPTX_MAX + 1; ++pdptx) {
			char *const vaddr_pdpt = (char *)KVADDR(pml4x, pdptx, 0, 0, 0);
			arch_pdpte_t *vpdpte = &mm_kernel_initial_pdpt[(PML4X(vaddr_pdpt) - PML4X(KERNEL_VBASE)) * 512 + pdptx];

			*vpdpte =
				ARCH_PDPTE_WITH_XD(
					ARCH_PDPTE_WITH_ADDR(
						ARCH_PDPTE_WITH_MASKS(0, PDPTE_P | PDPTE_RW | PDPTE_U),
						PGROUNDDOWN(
							((arch_pde_t *)mm_kernel_initial_pdt_paddr) +
							((pml4x - PML4X(KERNEL_VBASE)) * 512 * 512 +
								(pdptx - (PDPT_MATCH_INITIAL_COND ? PDPTX(KERNEL_VBASE) : 0)) * 512))),
					false);

			// Fill the Page Directory Table.

#define PDT_MATCH_INITIAL_COND (pml4x == PML4X(KERNEL_VBASE) && \
								pdptx == PDPTX(KERNEL_VBASE))
			for (uint32_t pdx = (PDT_MATCH_INITIAL_COND
									 ? PDX(KERNEL_VBASE)
									 : 0);
				pdx < PDX_MAX + 1; ++pdx) {
				char *const vaddr_pd = (char *)KVADDR(pml4x, pdptx, pdx, 0, 0);
				arch_pde_t *vpde = &mm_kernel_initial_pdt[(
					((pml4x - PML4X(KERNEL_VBASE)) * 512 * 512 +
						(pdptx - (PDPT_MATCH_INITIAL_COND
										 ? PDPTX(KERNEL_VBASE)
										 : 0)) *
							512 +
						pdx))];

				*vpde =
					ARCH_PDE_WITH_XD(
						ARCH_PDE_WITH_ADDR(
							ARCH_PDE_WITH_MASKS(0, PDE_P | PDE_RW | PDE_U),
							PGROUNDDOWN(
								((arch_pte_t *)mm_kernel_initial_ptt_paddr) +
								(((pml4x - PML4X(KERNEL_VBASE)) *
										512 * 512 * 512 +
									(pdptx - (PDPT_MATCH_INITIAL_COND
													 ? PDPTX(KERNEL_VBASE)
													 : 0)) *
										512 * 512 +
									(pdx - (PDT_MATCH_INITIAL_COND
												   ? PDX(KERNEL_VBASE)
												   : 0)) *
										512)))),
						false);

				if ((((uintptr_t)KVADDR(pml4x, pdptx, pdx, PTX_MAX, PGOFF_MAX)) < KERNEL_VBASE))
					continue;

				// Fill the Page Table Table.

#define PTT_MATCH_INITIAL_COND (pml4x == PML4X(KERNEL_VBASE) && \
								pdptx == PDPTX(KERNEL_VBASE) && \
								pdx == PDX(KERNEL_VBASE))
				for (uint32_t ptx = (PTT_MATCH_INITIAL_COND
										 ? PTX(KERNEL_VBASE)
										 : 0);
					ptx < PTX_MAX + 1;
					++ptx) {
					char *const vaddr_pt = (char *)KVADDR(pml4x, pdptx, pdx, ptx, 0);
					if ((((uintptr_t)vaddr_pt) < KERNEL_VBASE))
						continue;
					if (((((uintptr_t)vaddr_pt) >= (uintptr_t)mm_kernel_init_tmpmap_area)) &&
						((((uintptr_t)vaddr_pt) < (uintptr_t)&mm_kernel_init_tmpmap_area[PBOS_ARRAYSIZE(mm_kernel_init_tmpmap_area)])))
						continue;
					if (((uintptr_t)vaddr_pt) >= (uintptr_t)KVADDR(PML4X_MAX, PDPTX_MAX, PDX_MAX, PTX_MAX - 1, 0))
						goto fill_end;
					arch_pte_t *vpte = &mm_kernel_initial_ptt[(
																  ((pml4x - PML4X(KERNEL_VBASE)) *
																		  512 * 512 * 512 +
																	  (pdptx - (PDPT_MATCH_INITIAL_COND ? PDPTX(KERNEL_VBASE)
																										: 0)) *
																		  512 * 512 +
																	  (pdx - (PDT_MATCH_INITIAL_COND ? PDX(KERNEL_VBASE)
																									 : 0)) *
																		  512)) +
															  ptx];

					*vpte =
						ARCH_PTE_WITH_XD(
							ARCH_PTE_WITH_ADDR(
								ARCH_PTE_WITH_MASKS(0, PTE_P | PTE_RW),
								PGROUNDDOWN(
									(((char *)hali_limine_executable_address_request.response->physical_base)) +
									(((uintptr_t)vaddr_pt) - KERNEL_VBASE))),
							false);
				}
			}
		}
	}

fill_end:
	// Load PDT.
	arch_lpgtab(PGROUNDDOWN(mm_kernel_initial_pml4t_paddr));

	ki_cur_paging_config = &KI_PAGING_CONFIG_48BIT;
}

///
/// @brief Push a PMAD to the list. The descriptor will be copied.
///
/// @param pmad PMAD to push.
///
PBOS_NO_ASAN static void hali_push_pmad(ki_pmad_t &&pmad) {
	if (ki_pmad_number + 1 >= PBOS_ARRAYSIZE(ki_initial_pmad_storage))
		km_panic("Too many memory map entries");
	if (auto d = ki_pmad_tree.find_max_lteq(pmad.rb_value);
		d && (((char *)d->rb_value) + static_cast<ki_pmad_t *>(d)->len > pmad.rb_value))
		km_panic("Overlapped memory regions detected");
	ki_initial_pmad_storage[ki_pmad_number] = std::move(pmad);
	ki_pmad_tree.insert_unwrap(&ki_initial_pmad_storage[ki_pmad_number]);
	++ki_pmad_number;
}

PBOS_EXTERN_C_END

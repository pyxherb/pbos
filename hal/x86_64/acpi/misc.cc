#include <hal/x86_64/misc.h>
#include <pbos/kd/logger.h>
#include <string.h>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/acpi/rsdt.hh>

bool kh_acpi_is_available() {
	return hali_limine_rsdp_request.response;
}

bool kh_acpi_is_required() {
	return true;
}

void kh_acpi_init() {
	dbg_printf("Initializing ACPI...\n");

	const size_t page_size = mm_get_page_size();

	bool is_acpi_rsdp_direct_mmap = true;
	if (!(ki_acpi_rsdp_vaddr = (acpi_rsdp_t *)kh_get_direct_mmap(ki_acpi_rsdp_paddr))) {
		is_acpi_rsdp_direct_mmap = false;
		if (!(ki_acpi_rsdp_vaddr = (acpi_rsdp_t *)mm_kvmalloc(mm_get_cur_context(), page_size, MM_PAGE_MAPPED, 0)))
			km_panic("Error allocating virtual memory area for ACPI RSDP");
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), ki_acpi_rsdp_vaddr, ki_acpi_rsdp_paddr, page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			km_panic("Error mapping ACPI RSDP");
	}

	if (!acpi_verify_checksum((char *)ki_acpi_rsdp_vaddr, sizeof(acpi_rsdp_t)))
		km_panic("ACPI RSDP damaged");

	bool is_acpi_rsdt_direct_mmap = true;
	void *rsdt_paddr;
	switch (ki_acpi_rsdp_vaddr->revision) {
		case 0: {
			rsdt_paddr = (void *)(uintptr_t)ki_acpi_rsdp_vaddr->rsdt_addr;
			break;
		}
		case 2: {
			rsdt_paddr = (void *)((acpi_xsdp_t *)ki_acpi_rsdp_vaddr)->xsdt_addr;
			if (!is_acpi_rsdp_direct_mmap) {
				uint32_t len = ((acpi_xsdp_t *)ki_acpi_rsdp_vaddr)->length;
				mm_unmmap(mm_get_cur_context(), ki_acpi_rsdp_vaddr, page_size, 0);
				if (!(ki_acpi_rsdp_vaddr = (acpi_rsdp_t *)mm_kvmalloc(mm_get_cur_context(), len, MM_PAGE_MAPPED, 0)))
					km_panic("Error allocating virtual memory area for ACPI XSDP");
				if (KM_FAILED(mm_mmap(mm_get_cur_context(), ki_acpi_rsdp_vaddr, ki_acpi_rsdp_paddr, len, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
					km_panic("Error mapping ACPI XSDP");
			}
			if (!acpi_verify_checksum((char *)ki_acpi_rsdp_vaddr, ((acpi_xsdp_t *)ki_acpi_rsdp_vaddr)->length))
				km_panic("ACPI XSDP damaged");
			break;
		}
		default:
			km_panic("Invalid ACPI RSDP revision: %d", (int)ki_acpi_rsdp_vaddr->revision);
	}
	dbg_printf("Found ACPI RSDP with revision: %d\n", (int)ki_acpi_rsdp_vaddr->revision);

	if (!(ki_acpi_rsdt_vaddr = (acpi_sdt_header_t *)kh_get_direct_mmap(rsdt_paddr))) {
		is_acpi_rsdt_direct_mmap = false;
		if (!(ki_acpi_rsdt_vaddr = (acpi_sdt_header_t *)mm_kvmalloc(mm_get_cur_context(), page_size, MM_PAGE_MAPPED, 0)))
			km_panic("Error allocating virtual memory area for ACPI RSDT");
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), ki_acpi_rsdt_vaddr, (void *)(uint64_t)ki_acpi_rsdp_vaddr->rsdt_addr, page_size, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			km_panic("Error mapping ACPI RSDT");
	}

	char oem_id[7];
	memcpy(oem_id, ki_acpi_rsdt_vaddr->oem_id, sizeof(oem_id) - 1);
	oem_id[6] = '\0';
	dbg_printf("Found ACPI RSDT with OEM id: %s\n", oem_id);

	if (!is_acpi_rsdt_direct_mmap) {
		uint32_t len = ki_acpi_rsdt_vaddr->length;
		mm_unmmap(mm_get_cur_context(), ki_acpi_rsdt_vaddr, page_size, 0);
		if (!(ki_acpi_rsdt_vaddr = (acpi_sdt_header_t *)mm_kvmalloc(mm_get_cur_context(), len, MM_PAGE_MAPPED, 0)))
			km_panic("Error reallocating virtual memory area for ACPI RSDT");
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), ki_acpi_rsdt_vaddr, rsdt_paddr, len, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			km_panic("Error remapping ACPI RSDT");
	}

	if (!acpi_verify_checksum((char *)ki_acpi_rsdt_vaddr, ki_acpi_rsdt_vaddr->length))
		km_panic("ACPI RSDT damaged");

	dbg_printf("Mapping ACPI root tables...\n");

	const size_t rsdt_len = acpi_rsdt_length();

	if (rsdt_len) {
		if (!(ki_mapped_acpi_rsdt_entries = (acpi_sdt_header_t **)mm_kalloc(rsdt_len * sizeof(void *), alignof(void *))))
			km_panic("Error mapping ACPI root tables");

		for (size_t i = 0; i < rsdt_len; ++i) {
			bool is_direct_map = true;
			void *paddr = acpi_rsdt_paddr_at(i);

			char *vaddr_base;
			if (!(vaddr_base = (char *)kh_get_direct_mmap((void *)PGFLOOR(paddr)))) {
				is_direct_map = false;
				if (!(vaddr_base = (char *)mm_kvmalloc(mm_get_cur_context(), i, MM_PAGE_READ | MM_PAGE_WRITE, 0)))
					km_panic("Error mapping ACPI root tables");
				km_unwrap_result(mm_mmap(mm_get_cur_context(), vaddr_base, paddr, page_size, MM_PAGE_READ | MM_PAGE_WRITE, 0));
			}

			size_t off = ((uintptr_t)paddr) % page_size;
			acpi_sdt_header_t *vaddr = (acpi_sdt_header_t *)(vaddr_base + off);

			if (!(is_direct_map)) {
				uint32_t len = vaddr->length;
				mm_unmmap(mm_get_cur_context(), vaddr_base, page_size, 0);
				if (!(vaddr_base = (char *)mm_kvmalloc(mm_get_cur_context(), len, MM_PAGE_MAPPED, 0)))
					km_panic("Error reallocating virtual memory area for ACPI RSDT");
				if (KM_FAILED(mm_mmap(mm_get_cur_context(), vaddr_base, paddr, len, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
					km_panic("Error remapping ACPI RSDT");
				vaddr = (acpi_sdt_header_t *)(vaddr_base + off);
			}

			ki_mapped_acpi_rsdt_entries[i] = vaddr;
		}
	}

	dbg_printf("Initialized ACPI\n");
}

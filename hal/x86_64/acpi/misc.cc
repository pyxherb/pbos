#include <hal/x86_64/misc.h>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/mm.hh>
#include <pbos/kn/acpi/rsdt.hh>
#include <pbos/km/logger.h>

bool kh_acpi_is_available() {
	return hn_limine_rsdp_request.response;
}

bool kh_acpi_is_required() {
	return true;
}

void kh_acpi_init() {
	kd_printf("Initializing ACPI...\n");

	if (!(kn_acpi_rsdp_vaddr = (acpi_rsdp_t *)kh_get_direct_mmap(kn_acpi_rsdp_paddr))) {
		if (!(kn_acpi_rsdp_vaddr = (acpi_rsdp_t *)mm_kvmalloc(mm_get_cur_context(), PAGESIZE, MM_PAGE_MAPPED, 0)))
			km_panic("Error allocating virtual memory area for ACPI RSDP");
		if (KM_FAILED(mm_mmap(mm_get_cur_context(), kn_acpi_rsdp_vaddr, kn_acpi_rsdp_paddr, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
			km_panic("Error mapping ACPI RSDP");
	}

	if (!kn_acpi_verify_checksum((char *)kn_acpi_rsdp_vaddr, sizeof(acpi_rsdp_t)))
		km_panic("ACPI RSDP damaged");
	switch (kn_acpi_rsdp_vaddr->revision) {
		case 0:
			break;
		case 2: {
			if (!kn_acpi_verify_checksum((char *)kn_acpi_rsdp_vaddr, ((acpi_xsdp_t*)kn_acpi_rsdp_vaddr)->length))
				km_panic("ACPI XSDP damaged");
			break;
		}
		default:
			km_panic("Invalid ACPI RSDP revision: %d", (int)kn_acpi_rsdp_vaddr->revision);
	}
	kd_printf("Found ACPI RSDP with revision: %d\n", (int)kn_acpi_rsdp_vaddr->revision);

	kd_printf("Initialized ACPI\n");
}

#include <arch/x86_64/apic.h>
#include <pbos/acpi/madt.h>
#include <pbos/kd/logger.h>
#include <string.h>
#include <hal/x86_64/irq.hh>
#include <hal/x86_64/proc.hh>
#include <hal/x86_64/mm.hh>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/ki/mp/init.hh>
#include <hal/x86_64/mm/vmmgr/vm.hh>

PBOS_EXTERN_C_BEGIN

uint32_t hn_sched_interval;

arch_tss_t *hn_tss_storage_ptr;

hn_tmpmap_info_t *hn_tmpmap_storage_ptr;

hn_kgdt_t *hn_gdt_storage_ptr;

static void hn_mask_pic(uint8_t pic1_offset, uint8_t pic2_offset) {
	uint8_t pic1_mask = arch_in8(ARCH_PIC1_IO_DATA), pic2_mask = arch_in8(ARCH_PIC2_IO_DATA);

	// Initialize both PICs.
	arch_out8(ARCH_PIC1_IO_COMMAND, ARCH_ICW1_COMMAND_INIT | ARCH_ICW1_COMMAND_ICW4);
	arch_io_wait();
	arch_out8(ARCH_PIC2_IO_COMMAND, ARCH_ICW1_COMMAND_INIT | ARCH_ICW1_COMMAND_ICW4);
	arch_io_wait();

	// Set master PIC vector offset.
	arch_out8(ARCH_PIC1_IO_DATA, pic1_offset);
	arch_io_wait();

	// Set slave PIC vector offset.
	arch_out8(ARCH_PIC2_IO_DATA, pic2_offset);
	arch_io_wait();

	arch_out8(ARCH_PIC1_IO_DATA, 4);
	arch_io_wait();

	arch_out8(ARCH_PIC2_IO_DATA, 2);
	arch_io_wait();

	// Tell the PICs to use 8086 mode instead of 8080 mode.
	arch_out8(ARCH_PIC1_IO_DATA, ARCH_ICW4_COMMAND_8086);
	arch_io_wait();
	arch_out8(ARCH_PIC2_IO_DATA, ARCH_ICW4_COMMAND_8086);
	arch_io_wait();

	// Restore saved masks.
	arch_out8(ARCH_PIC1_IO_DATA, pic1_mask);
	arch_out8(ARCH_PIC2_IO_DATA, pic2_mask);

	arch_disable_pic();
}

void kh_mp_init_topology() {
	if (kh_acpi_is_available()) {
		uint32_t lapic_num = 0, lx2apic_num = 0;
		const size_t num_rsd = acpi_rsdt_length();
		for (size_t i = 0; i < num_rsd; ++i) {
			acpi_sdt_header_t *sdt = acpi_rsdt_vaddr_at(i);

			if (!memcmp(sdt->signature, "APIC", sizeof(sdt->signature))) {
				acpi_madt_exdata_t *exdata = (acpi_madt_exdata_t *)&sdt[1];

				const char *ptr = (char *)(&exdata[1]);

				while (ptr - (char *)sdt < (size_t)sdt->length) {
					const acpi_madt_entry_header_t *header = (const acpi_madt_entry_header_t *)ptr;

					switch (header->entry_type) {
						case ACPI_MADT_TYPE_LAPIC: {
							const acpi_madt_entry_lapic_t *hdr = (const acpi_madt_entry_lapic_t *)header;
							klog_printf("Found CPU #%u in LAPIC entry\n", (uint32_t)hdr->processor_id);
							++lapic_num;
							break;
						}
						case ACPI_MADT_TYPE_LX2APIC: {
							const acpi_madt_entry_lx2apic_t *hdr = (const acpi_madt_entry_lx2apic_t *)header;
							klog_printf("Found CPU in LX2APIC entry, id = %u\n", hdr->x2apic_id);
							++lx2apic_num;
							break;
						}
						default:
							break;
					}

					ptr += header->record_length;
				}

				break;
			}
		}

		if ((!lapic_num) && (!lx2apic_num))
			km_panic("The ACPI root tables does not contain MADT table");

		if (lx2apic_num)
			mp_num_total_cpu = lx2apic_num;
		else
			mp_num_total_cpu = lapic_num;

		klog_printf("Found %u CPUs\n", mp_num_total_cpu);
	} else {
		km_panic("Unable to detect CPU topology");
	}
}

void kh_mp_alloc_platform_resources() {
	hn_tss_storage_ptr = (arch_tss_t *)mm_kalloc(mp_num_total_cpu * sizeof(arch_tss_t), alignof(std::max_align_t));
	if (!hn_tss_storage_ptr) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	memset(hn_tss_storage_ptr, 0, mp_num_total_cpu * sizeof(arch_tss_t));

	hn_gdt_storage_ptr = (hn_kgdt_t *)mm_kalloc(mp_num_total_cpu * sizeof(hn_kgdt_t), alignof(std::max_align_t));
	if (!hn_gdt_storage_ptr) {
		km_panic("Unable to allocate memory for TSS storage for processors");
	}
	for (size_t i = 0; i < mp_num_total_cpu; ++i) {
		memcpy(&hn_gdt_storage_ptr[i], &hn_init_kgdt, sizeof(hn_kgdt_t));
		hn_gdt_storage_ptr[i].tss_desc1 =
			TSSDESC_LOW(((uint32_t)(uintptr_t)(hn_tss_storage_ptr + i)), sizeof(arch_tss_t), GDT_AB_P | GDT_AB_DPL(0) | GDT_SYSTYPE_TSS32, 0);
		hn_gdt_storage_ptr[i].tss_desc2 = TSSDESC_HIGH(((uintptr_t)(hn_tss_storage_ptr + i)));
	}

	hn_tmpmap_info_t *tmp_hn_tmpmap_storage_ptr = (hn_tmpmap_info_t *)mm_kalloc(mp_num_total_cpu * sizeof(hn_tmpmap_info_t), alignof(std::max_align_t));
	if (!tmp_hn_tmpmap_storage_ptr) {
		km_panic("Unable to allocate memory for TMPMAP information storage for processors");
	}

	for (size_t i = 0; i < mp_num_total_cpu; ++i) {
		void *vaddr = mm_kvmalloc(mm_get_cur_context(), KINITTMPMAP_SIZE, MM_PAGE_MAPPED, 0);

		if (!vaddr)
			km_panic("Unable to allocate TMPMAP space for TMPMAP information storage for processors");

		tmp_hn_tmpmap_storage_ptr[i].tmpmap_base = vaddr;
		kd_dbgcheck(
			(tmp_hn_tmpmap_storage_ptr[i].tmpmap_pgtab_base = (arch_pte_t*)hn_get_pgtab_paddr(mm_get_cur_context(), vaddr, nullptr)),
			"The TMPMAP page table address should not be invalid after mapped");
	}

	hn_tmpmap_storage_ptr = tmp_hn_tmpmap_storage_ptr;
}

void hn_calibrate_apic() {
	// Relocate and remap APIC.
	if (!(hn_lapic_vbase = (uint32_t *)mm_kvmalloc(mm_kernel_context, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE, 0)))
		km_panic("Unable to allocate virtual LAPIC page");

	arch_set_apic_base((void*)ARCH_DEFAULT_APIC_PBASE, ARCH_APIC_BASE_MSR_BSP | ARCH_APIC_BASE_MSR_ENABLE);

	if (KM_FAILED(mm_iommap(mm_kernel_context, hn_lapic_vbase, (void*)ARCH_DEFAULT_APIC_PBASE, PAGESIZE, MM_PAGE_MAPPED | MM_PAGE_READ | MM_PAGE_WRITE | MM_PAGE_NOCACHE, 0))) {
		km_panic("Unable to mapping LAPIC page for the main CPU");
	}

	uint8_t lapic_intvec = arch_read_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_SPURIOUS_INT_VEC) & 0xff;

	arch_cli();

	// Enable LAPIC
	arch_write_lapic(
		hn_lapic_vbase,
		ARCH_LAPIC_REG_SPURIOUS_INT_VEC,
		lapic_intvec | ARCH_LAPIC_SPURIOUS_INT_VEC_REG_ENABLE);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_DIVIDE_CONFIG, ARCH_LAPIC_DIVIDE_CONFIG_16);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, ARCH_LAPIC_LVT_TIMER_REG_ONESHOT);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0xffffffff);

	// Delay for 1000 microseconds to calibrate the APIC.
	arch_out8(0x43, 0x30);

	constexpr uint16_t COUNT_RATE = 11931;
	arch_out8(0x40, COUNT_RATE & 0xff);
	arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

	while (true) {
		arch_out8(0x43, 0x00);
		uint16_t cur_count = arch_in8(0x40);
		cur_count |= arch_in8(0x40) << 8;

		if (!cur_count)
			break;
	}

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, ARCH_LAPIC_LVT_TIMER_REG_MASKED);
	uint32_t cur_timer = arch_read_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_CURRENT_COUNT);
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0);
	hn_sched_interval = (0xffffffff - cur_timer) / 10;
	// hn_sched_interval = 1234;

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	arch_sti();
}

void mp_main_cpu_init() {
	arch_lgdt(&hn_gdt_storage_ptr[0], sizeof(hn_kgdt_t) / sizeof(arch_gdt_desc_t));
	arch_ltr(SELECTOR_TSS);

	arch_loades(SELECTOR_KDATA);

	hn_calibrate_apic();

	klog_printf("Timer interval ticks: %u\n", hn_sched_interval);
}

PBOS_EXTERN_C_END

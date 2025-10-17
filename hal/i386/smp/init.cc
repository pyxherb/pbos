#include <arch/i386/apic.h>
#include <pbos/kn/km/smp.h>
#include <pbos/km/logger.h>
#include "../irq.h"

PBOS_EXTERN_C_BEGIN

uint32_t hn_sched_interval;

void smp_init() {
	smp_main_eu_init();
}

void smp_main_eu_init() {
	arch_cli();

	hal_irq_context_t *ctxt = hal_irq_contexts[0];

	if (!(hn_lapic_pbase = mm_pgalloc(MM_PMEM_AVAILABLE)))
		km_panic("Unable to allocate physical LAPIC page");

	if (!(hn_lapic_vbase = (uint32_t *)mm_kvmalloc(mm_kernel_context, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE, 0)))
		km_panic("Unable to allocate virtual LAPIC page");

	arch_set_lapic_base(hn_lapic_pbase, ARCH_APIC_BASE_MSR_BSP);

	if (KM_FAILED(mm_mmap(mm_kernel_context, hn_lapic_vbase, hn_lapic_pbase, PAGESIZE, PAGE_MAPPED | PAGE_READ | PAGE_WRITE | PAGE_NOCACHE, 0))) {
		km_panic("Unable to mapping LAPIC page for the main EU");
	}

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_DIVIDE_CONFIG, 0x03);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, (1 << 16) | (0b00 << 17));

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0xffffffff);

	arch_enable_apic(hn_lapic_vbase);

	// Delay for 1000 microseconds to calibrate the APIC.
	arch_out8(0x43, 0x30);

	constexpr uint16_t COUNT_RATE = 1193;
	arch_out8(0x40, COUNT_RATE & 0xff);
	arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

	while (true) {
		arch_out8(0x43, 0x00);
		uint16_t cur_count = arch_in8(0x40);
		cur_count |= arch_in8(0x40) << 8;

		if (!cur_count)
			break;
	}

	uint32_t cur_timer = arch_read_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_CURRENT_COUNT);
	hn_sched_interval = 0xffffffff - cur_timer;

	kprintf("Timer interval ticks: %u\n", hn_sched_interval);
}

PBOS_EXTERN_C_END

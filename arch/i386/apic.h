#ifndef _ARCH_I386_APIC_H_
#define _ARCH_I386_APIC_H_

#include "msr.h"

PBOS_EXTERN_C_BEGIN

#define ARCH_APIC_BASE_MSR 0x1B

#define ARCH_APIC_BASE_MSR_BSP 0x100
#define ARCH_APIC_BASE_MSR_ENABLE 0x800

#define ARCH_LAPIC_PBASE 0xfee00000

#define ARCH_LAPIC_REG_ID_REG 0x020
#define ARCH_LAPIC_REG_VER_REG 0x030
#define ARCH_LAPIC_REG_TPR 0x080
#define ARCH_LAPIC_REG_APR 0x090
#define ARCH_LAPIC_REG_PPR 0x0a0
#define ARCH_LAPIC_REG_EOI 0x0b0
#define ARCH_LAPIC_REG_RRD 0x0c0
#define ARCH_LAPIC_REG_LOGICAL_DEST 0x0d0
#define ARCH_LAPIC_REG_DEST_FMT 0x0e0
#define ARCH_LAPIC_REG_SPURIOUS_INT_VEC 0x0f0

#define ARCH_LAPIC_REG_ISR_BEGIN 0x100
#define ARCH_LAPIC_REG_ISR_END 0x17f

#define ARCH_LAPIC_REG_TMR_BEGIN 0x180
#define ARCH_LAPIC_REG_TMR_END 0x1ff

#define ARCH_LAPIC_REG_IRR_BEGIN 0x200
#define ARCH_LAPIC_REG_IRR_END 0x27f

#define ARCH_LAPIC_REG_ERROR_STATUS 0x280

#define ARCH_LAPIC_REG_LVT_CMCI 0x2f0

#define ARCH_LAPIC_REG_ICR_BEGIN 0x300
#define ARCH_LAPIC_REG_ICR_END 0x31f

#define ARCH_LAPIC_REG_LVT_TIMER 0x320
#define ARCH_LAPIC_REG_LVT_THERMAL_SENSOR 0x330
#define ARCH_LAPIC_REG_LVT_PERF_MONITORING_COUNTER 0x340
#define ARCH_LAPIC_REG_LVT_LINT0 0x350
#define ARCH_LAPIC_REG_LVT_LINT1 0x360
#define ARCH_LAPIC_REG_LVT_ERROR 0x370

#define ARCH_LAPIC_REG_INITIAL_COUNT 0x380
#define ARCH_LAPIC_REG_CURRENT_COUNT 0x390
#define ARCH_LAPIC_REG_DIVIDE_CONFIG 0x3e0

#define ARCH_LAPIC_SPURIOUS_INT_VEC_REG_ENABLE 0x100

#define ARCH_LAPIC_LVT_TIMER_REG_MASKED (1 << 16)
#define ARCH_LAPIC_LVT_TIMER_REG_ONESHOT (0b00 << 17)
#define ARCH_LAPIC_LVT_TIMER_REG_PERIODIC (0b01 << 17)

PBOS_FORCEINLINE static bool arch_has_apic() {
	uint32_t a, b, c, d;
	arch_cpuid(1, &a, &b, &c, &d);
	return d & ARCH_CPUID_FEATURE_EDX_APIC;
}

PBOS_FORCEINLINE void arch_set_lapic_base(void *base, uint32_t flags) {
	uint32_t edx = 0;
	uint32_t eax = ((((uintptr_t)base) & 0xfffff000) | ARCH_APIC_BASE_MSR_ENABLE | flags);

	arch_wrmsr(ARCH_APIC_BASE_MSR, eax, edx);
}

PBOS_FORCEINLINE void *arch_get_lapic_base() {
	uint32_t eax, edx;
	arch_rdmsr(ARCH_APIC_BASE_MSR, &eax, &edx);

	return (void *)(eax & 0xfffff000);
}

PBOS_FORCEINLINE uint32_t arch_read_io_apic(void *io_apic_addr, uint32_t reg) {
	uint32_t volatile *io_apic = (uint32_t volatile *)io_apic_addr;
	io_apic[0] = (reg & 0xff);
	return io_apic[4];
}

PBOS_FORCEINLINE void arch_write_io_apic(void *io_apic_addr, uint32_t reg, uint32_t value) {
	uint32_t volatile *io_apic = (uint32_t volatile *)io_apic_addr;
	io_apic[0] = (reg & 0xff);
	io_apic[4] = value;
}

PBOS_FORCEINLINE uint32_t arch_read_lapic(void *base, uint32_t reg) {
	uint32_t volatile *lapic = (uint32_t volatile *)base;

	return *(lapic + (reg >> 2));
}

PBOS_FORCEINLINE void arch_write_lapic(void *base, uint32_t reg, uint32_t value) {
	uint32_t volatile *lapic = (uint32_t volatile *)base;

	*(lapic + (reg >> 2)) = value;
}

PBOS_FORCEINLINE void arch_set_lapic_timer_divisor(void *base, uint32_t divisor) {
	uint32_t volatile *lapic = (uint32_t volatile *)base;

	*(lapic + ARCH_LAPIC_REG_DIVIDE_CONFIG) = divisor & 0xf;
}

PBOS_FORCEINLINE void arch_set_lapic_timer_mode(void *base, uint8_t vec, uint8_t mode) {
	uint32_t volatile *lapic = (uint32_t volatile *)base;

	*(lapic + ARCH_LAPIC_REG_LVT_TIMER) = vec | (mode << 17);
}

PBOS_EXTERN_C_END

#endif

#ifndef _ARCH_I386_REG_H_
#define _ARCH_I386_REG_H_

#include <pbos/common.h>

#define CR0_PE 0x00000001  // Protected mode enable
#define CR0_MP 0x00000002  // Monitor coprocessor
#define CR0_EM 0x00000004  // x87 emulation
#define CR0_TS 0x00000008  // Task switched
#define CR0_ET 0x00000010  // Extension type
#define CR0_NE 0x00000020  // Numeric error
#define CR0_WP 0x00010000  // Write-protect
#define CR0_AM 0x00040000  // Alignment mask
#define CR0_NW 0x20000000  // Not-Write through
#define CR0_CD 0x40000000  // Cache disable
#define CR0_PG 0x80000000  // Paging

#define CR4_VME 0x00000001		   // Virtual 8086 mode extensions
#define CR4_PVI 0x00000002		   // Protected mode virtual interrupts
#define CR4_TSD 0x00000004		   // Time stamp disable
#define CR4_DE 0x00000008		   // Debugging extensions
#define CR4_PSE 0x00000010		   // Page sie extension
#define CR4_PAE 0x00000020		   // Physical address extension
#define CR4_MCE 0x00000040		   // Machine check exception
#define CR4_PGE 0x00000080		   // Page global enable
#define CR4_PCE 0x00000100		   // Performance-monitoring counter enable
#define CR4_OSFXSR 0x00000200	   // OS support for FXSAVE & FXRSTOR instructions
#define CR4_OSXMMEXCPT 0x00000400  // OS support for unmasked SIMD FP exceptions
#define CR4_UMIP 0x00000800		   // User-mode instruction prevention
#define CR4_VMXE 0x00002000		   // Virtual machine extensions enable
#define CR4_SMXE 0x00004000		   // Safer mode extensions enable
#define CR4_FSGSBASE 0x00010000	   // Enable RDFSBASE/RDGSBASE/WRFSBASE/WRGSBASE
#define CR4_PCIDE 0x00020000	   // PCID enable
#define CR4_OSXSAVE 0x00040000	   // XSAVE and processor extended states enable
#define CR4_SMEP 0x00100000		   // Supervisor mode execution protection enable
#define CR4_SMAP 0x00200000		   // Supervisor mode access prevention enable
#define CR4_PKE 0x00400000		   // Protection key enable
#define CR4_CET 0x00800000		   // Control-flow enforcement technology
#define CR4_PKS 0x01000000		   // Enable protection keys for supervisor-mode pages

///
/// @brief Read CR0 register.
///
/// @return The value stored in CR0 register.
///
PB_FORCEINLINE static uint32_t arch_rcr0() {
	uint32_t value;
	__asm__ __volatile__("movl %%cr0,%0"
						 : "=r"(value));
	return value;
}

///
/// @brief Write CR0 register.
///
/// @param value The value to write.
///
PB_FORCEINLINE static void arch_wcr0(uint32_t value) {
	__asm__ __volatile__("movl %0,%%cr0" ::"r"(value));
}

///
/// @brief Read CR2 register.
///
/// @return The value stored in CR2 register.
///
PB_FORCEINLINE static uint32_t arch_rcr2() {
	uint32_t value;
	__asm__ __volatile__("movl %%cr2,%0"
						 : "=r"(value));
	return value;
}

///
/// @brief Write CR2 register.
///
/// @param value The value to write.
///
PB_FORCEINLINE static void arch_wcr2(uint32_t value) {
	__asm__ __volatile__("movl %0,%%cr2" ::"r"(value));
}

///
/// @brief Read CR3 register.
///
/// @return The value stored in CR3 register.
///
PB_FORCEINLINE static uint32_t arch_rcr3() {
	uint32_t value;
	__asm__ __volatile__("movl %%cr3,%0"
						 : "=r"(value));
	return value;
}

///
/// @brief Write CR3 register.
///
/// @param value The value to write.
///
PB_FORCEINLINE static void arch_wcr3(uint32_t value) {
	__asm__ __volatile__("movl %0,%%cr3" ::"r"(value));
}

///
/// @brief Read CR4 register.
///
/// @return The value stored in CR4 register.
///
PB_FORCEINLINE static uint32_t arch_rcr4() {
	uint32_t value;
	__asm__ __volatile__("movl %%cr4,%0"
						 : "=r"(value));
	return value;
}

///
/// @brief Write CR4 register.
///
/// @param value The value to write.
///
PB_FORCEINLINE static void arch_wcr4(uint32_t value) {
	__asm__ __volatile__("movl %0,%%cr4" ::"r"(value));
}

///
/// @brief Read EFLAGS register.
///
/// @return The value stored in EFLAGS register.
///
PB_FORCEINLINE static uint32_t arch_rflags() {
	uint32_t eflags;
	__asm__ __volatile__("pushfl");
	__asm__ __volatile__("popl %0"
						 : "=r"(eflags));
	return eflags;
}

///
/// @brief Write EFLAGS register.
///
/// @param value The value to write.
///
PB_FORCEINLINE static void arch_wflags(uint32_t eflags) {
	__asm__ __volatile__("pushl %0" ::"r"(eflags));
	__asm__ __volatile__("popfl");
}

#define arch_rdmsr(msr, l, h) asm volatile("rdmsr" : "=a"(*(l)), "=d"(*(h)) : "c"(msr)))
#define arch_wrmsr(msr, l, h) asm volatile("wrmsr" :: "a"(l), "d"(h), "c"(msr)))

///
/// @brief Get value of EIP register.
///
/// @return Value of EIP register.
///
void* arch_reip();

#endif

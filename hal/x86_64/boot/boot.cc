#include <arch/x86_64/irq.h>
#include <arch/x86_64/misc.h>
#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <hal/x86_64/misc.h>
#include <hal/x86_64/mm.hh>

PBOS_EXTERN_C_BEGIN

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile uint64_t hali_limine_base_revision[] = HALI_REQUIRED_LIMINE_REVISION;

PBOS_USED PBOS_IN_SECTION(".limine_requests_start") volatile uint64_t hali_limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_framebuffer_request hali_limine_framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_hhdm_request hali_limine_hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 4
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_executable_address_request hali_limine_executable_address_request = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
	.revision = 1
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_memmap_request hali_limine_memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_rsdp_request hali_limine_rsdp_request = {
	.id = LIMINE_RSDP_REQUEST_ID,
	.revision = 4
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_module_request hali_limine_module_request = {
	.id = LIMINE_MODULE_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_request_end") volatile uint64_t hali_limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

PBOS_NORETURN void kernel_main();

PBOS_NO_ASAN void kmain() {
	// Setup SSE.
	uint64_t cr0 = arch_rcr0();
	cr0 &= ~CR0_EM;
	cr0 |= ~CR0_MP;
	arch_wcr0(cr0);

	uint64_t cr4 = arch_rcr4();
	cr4 |= CR4_OSXMMEXCPT;
	arch_wcr4(cr4);

	asm volatile("xorq %rbp,%rbp");
	asm volatile("movq %0, %%rsp" :: "r"(mm_kernel_initial_stack + PBOS_ARRAYSIZE(mm_kernel_initial_stack)): "memory");

	kernel_main();
}

PBOS_EXTERN_C_END

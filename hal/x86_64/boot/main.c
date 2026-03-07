#include <arch/x86_64/irq.h>
#include <arch/x86_64/misc.h>
#include <arch/x86_64/mlayout.h>
#include <arch/x86_64/paging.h>
#include <hal/x86_64/misc.h>
#include <hal/x86_64/mm.h>

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile uint64_t hn_limine_base_revision[] = HN_REQUIRED_LIMINE_REVISION;

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_framebuffer_request hn_limine_framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_hhdm_request hn_limine_hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 4
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_executable_address_request hn_limine_executable_address_request = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
	.revision = 1
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_memmap_request hn_limine_memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_requests") volatile struct limine_module_request hn_limine_module_request = {
	.id = LIMINE_MODULE_REQUEST_ID,
	.revision = 0
};

PBOS_USED PBOS_IN_SECTION(".limine_requests_start") volatile uint64_t hn_limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

PBOS_USED PBOS_IN_SECTION(".limine_request_end") volatile uint64_t hn_limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

PBOS_EXTERN_C PBOS_NORETURN void kernel_main();

void kmain() {
	asm volatile("xorq %rbp,%rbp");
	asm volatile("movabsq %0, %%rsp" ::"i"(mm_kernel_initial_stack + PBOS_ARRAYSIZE(mm_kernel_initial_stack)) : "memory");
	kernel_main();
}

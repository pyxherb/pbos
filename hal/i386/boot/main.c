#include <arch/i386/cpuid.h>
#include <arch/i386/misc.h>
#include <arch/i386/mlayout.h>
#include <arch/i386/paging.h>
#include <arch/i386/reg.h>
#include <hal/i386/display/vga.h>
#include <string.h>
#include "error.h"
#include "kloader.h"
#include "mb.h"
#include "misc.h"

/// @brief Initialize paging for the bootloader itself.
static void boot_init_paging();

/// @brief C main routine of the bootloader.
void boot_main() {
	vga_puts("Loading OICKIM...");

	if (boot_mb_magic != MB_BOOTLDR_MAGIC)
		boot_panic("Incompatible bootloader");

	if (boot_infoscan())
		boot_panic("Error scanning boot information:\n%s", boot_geterr());

	if (!(ARCH_KARGS_PTR->initcar_ptr))
		boot_panic("INITCAR was not loaded");

	boot_init_paging();

	if (!boot_load_pbkim())
		boot_panic("Error loading pbkim:\n%s", boot_geterr());

	vga_printf("Starting kernel at %p\n", boot_kentry);

	memset((void *)KSTACK_VBASE, 0, KSTACK_SIZE);
}

static void boot_init_paging() {
	arch_pde_t *pde = (arch_pde_t *)0x8000;		// Pointer to the PDE.
	arch_pte_t *curptr = (arch_pte_t *)0xa000;	// Current pointer for PTE allocation.

	memset(pde, 0, sizeof(arch_pde_t) * 1024);

	//
	// Create virtual memory mapping for the bootloader.
	// In this area, virtual address is the physical address.
	//
	for (uint32_t i = PDX(0x00000000); ((uint32_t)VADDR(i, 0, 0)) < KERNEL_PBASE; ++i) {
		pde[i].mask = PDE_P | PDE_RW;
		pde[i].address = PGROUNDDOWN(curptr);

		for (uint32_t k = 0; (k < 1024) && (((uint32_t)VADDR(i, k, PAGESIZE - 1)) < KERNEL_PBASE); ++k) {
			curptr[k].mask = PTE_P | PTE_RW;
			curptr[k].address = PGROUNDDOWN(VADDR(i, k, 0));
		}

		curptr += 1024;
	}

	//
	// Build the initial page table for the kernel.
	//
	for (uint32_t i = PDX(INIT_CRITICAL_VBASE), j = 0; ((uint32_t)VADDR(i, 0, 0)) <= INIT_CRITICAL_VTOP; ++i, j++) {
		pde[i].mask = PDE_P | PDE_RW;
		pde[i].address = PGROUNDDOWN(curptr);

		for (uint32_t k = 0; (k < 1024) && (((uint32_t)VADDR(i, k, PAGESIZE - 1)) <= INIT_CRITICAL_VTOP); ++k) {
			curptr[k].mask = PTE_P | PTE_RW;
			curptr[k].address = PGROUNDDOWN(INIT_CRITICAL_PBASE + ((uint32_t)VADDR(j, k, 0)));
		}

		curptr += 1024;
	}

	// Enable paging.
	arch_lpdt(PGROUNDDOWN(pde));
	arch_wcr0(arch_rcr0() | CR0_PG | CR0_PE);
}

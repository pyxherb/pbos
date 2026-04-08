#ifndef _ARCH_X86_64_PAGING_H_
#define _ARCH_X86_64_PAGING_H_

#include <pbos/common.h>
#include <stdint.h>
#include "memconf.h"

#define PML4E_P (1ULL << 0)	   // Present
#define PML4E_RW (1ULL << 1)   // Read/Write
#define PML4E_U (1ULL << 2)	   // User
#define PML4E_WT (1ULL << 3)   // Write-Through
#define PML4E_CD (1ULL << 4)   // Cache Disabled
#define PML4E_A (1ULL << 5)	   // Accessed

typedef struct PBOS_PACKED _arch_pml4te_t {
	uint16_t mask : 12;
	pgaddr_t address : 40;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_pml4te_t;

#define PDPTE_P (1ULL << 0)	   // Present
#define PDPTE_RW (1ULL << 1)   // Read/Write
#define PDPTE_U (1ULL << 2)	   // User
#define PDPTE_WT (1ULL << 3)   // Write-Through
#define PDPTE_CD (1ULL << 4)   // Cache Disabled
#define PDPTE_A (1ULL << 5)	   // Accessed
#define PDPTE_D (1ULL << 6)	   // Dirty
#define PDPTE_S (1ULL << 7)	   // Page Size
#define PDPTE_G (1ULL << 8)	   // Global
#define PDPTE_AT (1ULL << 12)  // Page Attribute

typedef struct PBOS_PACKED _arch_pdpte_t {
	uint16_t mask : 12;
	pgaddr_t address : 40;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_pdpte_t;

typedef struct PBOS_PACKED _arch_hugepg_pdpte_t {
	uint16_t mask : 13;
	uint32_t _reserved0 : 17;
	pgaddr_t address : 22;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_hugepg_pdpte_t;

#define PDE_P (1ULL << 0)	 // Present
#define PDE_RW (1ULL << 1)	 // Read/Write
#define PDE_U (1ULL << 2)	 // User
#define PDE_WT (1ULL << 3)	 // Write-Through
#define PDE_CD (1ULL << 4)	 // Cache Disabled
#define PDE_A (1ULL << 5)	 // Accessed
#define PDE_D (1ULL << 6)	 // Dirty
#define PDE_S (1ULL << 7)	 // Page Size
#define PDE_G (1ULL << 8)	 // Global
#define PDE_AT (1ULL << 12)	 // Page Attribute

typedef struct PBOS_PACKED _arch_pde_t {
	uint16_t mask : 12;
	pgaddr_t address : 40;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_pde_t;

typedef struct PBOS_PACKED _arch_hugepg_pde_t {
	uint16_t mask : 13;
	uint8_t _reserved0 : 8;
	pgaddr_t address : 31;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_hugepg_pde_t;

#define PTE_P 0x0001   // Present
#define PTE_RW 0x0002  // Read/Write
#define PTE_U 0x0004   // User
#define PTE_WT 0x0008  // Write-Through
#define PTE_CD 0x0010  // Cache Disabled
#define PTE_A 0x0020   // Accessed
#define PTE_D 0x0040   // Dirty
#define PTE_AT 0x0080  // Page Attribute Table
#define PTE_G 0x0080   // Global

typedef struct PBOS_PACKED _arch_pte_t {
	uint16_t mask : 12;
	pgaddr_t address : 40;
	uint8_t _reserved1 : 7;
	uint8_t pk : 4;
	bool xd : 1;
} arch_pte_t;

PBOS_FORCEINLINE void arch_invlpg(void *addr) {
	__asm__ __volatile__("invlpg (%0)" ::"b"((uint64_t)(addr))
		: "memory");
}
#define arch_lpgtab(paddr) \
	arch_wcr3((arch_rcr3() & ~0xfffffffffffff000LL) | (((uint32_t)(paddr)) << 12))
#define arch_spgtab() (arch_rcr3() & 0xfffffffffffff000)

#endif

#ifndef _PBOS_PCI_MCFG_H_
#define _PBOS_PCI_MCFG_H_

#include <pbos/acpi/rsdt.h>

typedef struct _pci_mcfg_entry_t {
	/// @brief Address of Enhanced Configuration Access Machanism (ECAM) space.
	uint64_t ecam_base;
	/// @brief PCI segment group number.
	uint16_t pci_segment_group_num;
	/// @brief Start PCI bus number which is decoded by this host bridge.
	uint8_t bus_start;
	/// @brief End PCI bus number which is decoded by this host bridge.
	uint8_t bus_end;
	uint32_t _reserved;
} pci_mcfg_entry_t;

#endif

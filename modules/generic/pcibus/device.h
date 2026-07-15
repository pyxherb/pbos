#ifndef _PCIROOT_DEVICE_H_
#define _PCIROOT_DEVICE_H_

#include "mcfg.h"

PBOS_EXTERN_C_BEGIN

enum {
	PCIBUS_ECAM_OFF_MAX = 0xfff,
	PCIBUS_FUNC_MAX = 0x7,
	PCIBUS_DEVICE_MAX = 0x1f,
	PCIBUS_BUS_MAX = 0xff,

	// Size per segment is 256MB.
	PCIBUS_ECAM_SIZE_PER_SEGMENT = ((PCIBUS_ECAM_OFF_MAX) | ((PCIBUS_FUNC_MAX) << 12) | (((PCIBUS_DEVICE_MAX) & 0x1f) << 15) | (PCIBUS_BUS_MAX << 20)) + 1
};

uint16_t pcibus_read_pci_config_space16(uint8_t bus, uint8_t device, uint8_t func, uint8_t off);

uint16_t pcibus_read_pcie_config_space16(pcibus_domain_t *domain, uint8_t bus, uint8_t device, uint8_t func, size_t off);
uint32_t pcibus_read_pcie_config_space32(pcibus_domain_t *domain, uint8_t bus, uint8_t device, uint8_t func, size_t off);

km_result_t pcibus_scan_devices_in_bus(pcibus_domain_t *domain, uint8_t bus);

PBOS_EXTERN_C_END

PBOS_FORCEINLINE constexpr size_t pcibus_calc_pcie_ecam_off(uint8_t bus, uint8_t device, uint8_t func, size_t off) {
	kd_assert(off <= PCIBUS_ECAM_OFF_MAX);
	kd_assert(func <= PCIBUS_FUNC_MAX);
	kd_assert(device <= PCIBUS_DEVICE_MAX);
	kd_assert(bus <= PCIBUS_BUS_MAX);
	return static_cast<size_t>((off & 0xfff) | ((func & 0x07) << 12) | ((device & 0x1f) << 15) | (bus << 20));
}

#endif

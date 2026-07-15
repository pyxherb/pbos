#include "device.h"
#include <pbos/generated/dm/devcls.h>
#include <pbos/io/ioport.h>
#include <pbos/kd/logger.h>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

uint16_t pcibus_read_pci_config_space16(uint8_t bus, uint8_t device, uint8_t func, uint8_t off) {
	io_write_ioport32(0xcf8,
		static_cast<uint32_t>(
			(bus << 16) | (device << 11) |
			(func << 8) | (off & 0xfc) | ((uint32_t)0x80000000)));

	return static_cast<uint16_t>(io_read_ioport32(0xcfc) >> ((off & 2) * 8));
}

uint16_t pcibus_read_pcie_config_space16(pcibus_domain_t *domain, uint8_t bus, uint8_t device, uint8_t func, size_t off) {
	if (!domain->ecam_vbase) {
		kd_assert(off <= 0xff - sizeof(uint16_t));
		return pcibus_read_pci_config_space16(bus, device, func, static_cast<uint8_t>(off));
	}
	kd_assert(off <= PCIBUS_ECAM_OFF_MAX - sizeof(uint16_t));
	return *static_cast<uint16_t *>(static_cast<void *>(static_cast<char *>(domain->ecam_vbase) + pcibus_calc_pcie_ecam_off(bus, device, func, off)));
}

uint32_t pcibus_read_pcie_config_space32(pcibus_domain_t *domain, uint8_t bus, uint8_t device, uint8_t func, size_t off) {
	if (!domain->ecam_vbase) {
		kd_assert(off <= 0xff - sizeof(uint32_t));
		uint8_t off8 = static_cast<uint8_t>(off);
		return static_cast<uint32_t>(pcibus_read_pci_config_space16(bus, device, func, off8)) | (static_cast<uint32_t>(pcibus_read_pci_config_space16(bus, device, func, off8)) << 16);
	}
	kd_assert(off <= PCIBUS_ECAM_OFF_MAX - sizeof(uint32_t));
	return *static_cast<uint32_t *>(static_cast<void *>(static_cast<char *>(domain->ecam_vbase) + pcibus_calc_pcie_ecam_off(bus, device, func, off)));
}

km_result_t pcibus_scan_devices_in_bus(pcibus_domain_t *domain, uint8_t bus) {

}

PBOS_EXTERN_C_END

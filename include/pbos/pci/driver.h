#ifndef _PBOS_PCI_DRIVER_H_
#define _PBOS_PCI_DRIVER_H_

#include "device.h"
#include <pbos/dm/device.h>

#define PCI_PCIBUS_KMOD_NAME "pbos.pcibus"

typedef struct _pci_device_id_t {
	uint16_t vendor_id, device_id, subsystem_vendor_id, subsystem_id;
} pci_device_id_t;

typedef struct _pci_device_exdata_t {
} pci_device_exdata_t;

typedef km_result_t (*pci_driver_found_op_t)(io_dispatch_context_t *dc, dm_device_t *device, const pci_device_id_t *device_id);

typedef struct _pci_driver_ops_t {
	pci_driver_found_op_t found;
} pci_driver_ops_t;

PBOS_API km_result_t pci_register_driver(const pci_device_id_t *device_ids, const pci_driver_ops_t *ops);
PBOS_API void pci_unregister_driver(const pci_device_id_t *device_id);

#endif

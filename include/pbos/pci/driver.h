#ifndef _PBOS_PCI_DRIVER_H_
#define _PBOS_PCI_DRIVER_H_

#include "device.h"
#include <pbos/dm/device.h>

#define PCI_PCIBUS_KMOD_NAME "pcibus"

//
// Match order:
// Vendor ID, Device ID, Subsystem Vendor ID, Subsystem ID
// Vendor ID, Device ID
// Class Code, Subclass, Prog IF
//
typedef struct _pci_device_id_t {
	uint16_t vendor_id, device_id, subsystem_vendor_id, subsystem_id;
} pci_device_id_t;

typedef struct _pci_device_class_t {
	uint8_t class_code, subclass, prog_if;
} pci_device_class_t;

typedef struct _pci_device_exdata_t {
} pci_device_exdata_t;

typedef km_result_t (*pci_driver_detected_op_t)(dm_device_t *device, const pci_device_id_t *device_id);
typedef void (*pci_driver_removed_op_t)(dm_device_t *device);

typedef struct _pci_driver_ops_t {
	pci_driver_detected_op_t detected;
} pci_driver_ops_t;

PBOS_KMOD_API km_result_t pci_register_device_driver(const pci_device_id_t *device_ids, const pci_driver_ops_t *ops);
PBOS_KMOD_API void pci_unregister_device_driver(const pci_device_id_t *device_id);

PBOS_KMOD_API km_result_t pci_register_class_driver(const pci_device_class_t *device_cls, const pci_driver_ops_t *ops);
PBOS_KMOD_API void pci_unregister_class_driver(const pci_device_class_t *device_cls);

#endif

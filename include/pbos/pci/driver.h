#ifndef _PBOS_PCI_DRIVER_H_
#define _PBOS_PCI_DRIVER_H_

#include "device.h"

#define PCI_DEVICE_MODEL_VENDOR_ID(m) ((m) & 0xffff)
#define PCI_DEVICE_MODEL_DEVICE_ID(m) (((m) >> 16) & 0xffff)
#define PCI_DEVICE_MODEL_SUBSYSTEM_VENDOR_ID(m) (((m) >> 32) & 0xffff)
#define PCI_DEVICE_MODEL_SUBSYSTEM_ID(m) (((m) >> 48) & 0xffff)

#define PCI_MAKE_DEVICE_MODEL(vendor_id, device_id, subsystem_vendor_id, subsystem_id) \
	((vendor_id & 0xffff) |\
	((device_id & 0xffff) << 16)|\
	((subsystem_vendor_id & 0xffff) << 32)|\
	((subsystem_id & 0xffff) << 48))

typedef uint64_t pci_device_model_t;

typedef struct _pci_driver_ops_t {

} pci_driver_ops_t;

#endif

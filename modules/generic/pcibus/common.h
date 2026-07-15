#ifndef _PCIROOT_COMMON_H_
#define _PCIROOT_COMMON_H_

#include <pbos/dm/bus.h>
#include <pbos/dm/device.h>
#include <pbos/kf/atomic.h>
#include <pbos/fs/file.hh>
#include <pbos/kfxx/map.hh>
#include <pbos/kfxx/string_view.hh>
#include <pbos/pci/driver.h>

PBOS_EXTERN_C_BEGIN

#define PCIROOT_COMPONENT_NAME "pcibus"

using pcibus_domain_id_t = uint32_t;

struct pcibus_domain_t;

struct pcibus_domain_t : public kfxx::rbtree_t<pcibus_domain_id_t>::node_t {
	size_t ref_count = 0;
	dm_device_t *device = nullptr;
	uint16_t segment_group_id = 0xffff;
	void *ecam_pbase = nullptr, *ecam_vbase = nullptr;

	pcibus_domain_t();
	~pcibus_domain_t();

	PBOS_FORCEINLINE void inc_ref() noexcept {
		kf_atomic_inc_size(&ref_count);
	}

	PBOS_FORCEINLINE void dec_ref() noexcept {
		if (!kf_atomic_dec_size(&ref_count)) {
			kfxx::destroy_and_release<pcibus_domain_t>(kfxx::kernel_allocator(), this);
		}
	}

	static pcibus_domain_t *alloc();
};

using pcibus_domain_ptr = kfxx::rc_object_ptr<pcibus_domain_t>;

struct pcibus_device_t : public kfxx::rbtree_t<pci_device_id_t>::node_t {
	size_t ref_count = 0;
	dm_device_t *device = nullptr;
	uint16_t segment_group_id = 0xffff;

	pcibus_device_t();

	PBOS_FORCEINLINE void inc_ref() noexcept {
		kf_atomic_inc_size(&ref_count);
	}

	PBOS_FORCEINLINE void dec_ref() noexcept {
		if (!kf_atomic_dec_size(&ref_count)) {
			kfxx::destroy_and_release<pcibus_device_t>(kfxx::kernel_allocator(), this);
		}
	}

	static pcibus_device_t *alloc();
};

extern kfxx::option_t<kfxx::map_t<uint16_t, pcibus_domain_ptr>> pcibus_segment_group_id_to_domain_map;
extern kfxx::rbtree_t<pcibus_domain_id_t> pcibus_domain_tree;
extern kfxx::rbtree_t<pci_device_id_t> pcibus_all_devices_set;

extern fs::fnode_ptr pcibus_devio_pci_root_dir;

constexpr kfxx::string_view PCIBUS_BUS_NAME = "pci";
constexpr kfxx::string_view PCIBUS_DEVIO_PCI_ROOT_DIR_NAME = "pci";

extern dm_bus_t *pcibus_bus_object;

/// @brief Allocate a new free domain ID.
///
/// @return Allocated domain ID, @c nullopt if failed.
bool pcibus_alloc_domain_id_and_insert(pcibus_domain_t *registry);

km_result_t pcibus_clear_domain_dir();

extern dm_device_class_t *pcibus_bus_controller_device_class;

km_result_t pcibus_fetch_device_classes_to_global_vars();

PBOS_EXTERN_C_END

#endif

#include "common.h"
#include "device.h"
#include <pbos/generated/dm/devcls.h>
#include <pbos/io/ioport.h>
#include <pbos/kd/logger.h>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

kfxx::option_t<kfxx::map_t<uint16_t, pcibus_domain_ptr>> pcibus_segment_group_id_to_domain_map;

kfxx::rbtree_t<pcibus_domain_id_t> pcibus_domain_tree;

fs::fnode_ptr pcibus_devio_pci_root_dir;

dm_bus_t *pcibus_bus_object = nullptr;

dm_device_class_t *pcibus_bus_controller_device_class;

pcibus_domain_t::pcibus_domain_t() {}

pcibus_domain_t::~pcibus_domain_t() {
	if (ecam_vbase) {
		km_unwrap_result(mm_munmap(mm_get_cur_context(), ecam_vbase, PCIBUS_ECAM_SIZE_PER_SEGMENT, 0));
	}
}

pcibus_domain_t *pcibus_domain_t::alloc() {
	return kfxx::alloc_and_construct<pcibus_domain_t>(kfxx::kernel_allocator());
}

static ps::mutex_t domain_id_alloc_mutex;

bool pcibus_alloc_domain_id_and_insert(pcibus_domain_t *registry) {
	ps::mutex_guard g(domain_id_alloc_mutex);

	if (pcibus_domain_tree.size() == UINT16_MAX)
		return false;

	if (pcibus_domain_tree.empty()) {
		registry->rb_value = 0;
		pcibus_domain_tree.insert_unwrap(registry);
		return true;
	}

	auto end = pcibus_domain_tree.end_const();
	auto it = pcibus_domain_tree.begin_const(), last_it = it;
	for (; it != end; ++it) {
		if ((*it) > (*last_it) + 1) {
			registry->rb_value = (*last_it) + 1;
			pcibus_domain_tree.insert_unwrap(registry);
			return true;
		}
		last_it = it;
	}

	registry->rb_value = (*last_it) + 1;
	pcibus_domain_tree.insert_unwrap(registry);
	return true;
}

km_result_t pcibus_clear_domain_dir() {
	km_result_t result;

	for (auto i : pcibus_domain_tree) {
		char name[sizeof("0000")] = {};

		name[0] = (i & 0xff) + '0';
		name[1] = ((i >> 4) & 0xff) + '0';
		name[2] = ((i >> 8) & 0xff) + '0';
		name[3] = ((i >> 12) & 0xff) + '0';

		// TODO: Implement it.

		dbg_println(PCIROOT_COMPONENT_NAME, "Removed directory for PCI domain: %s", name);
	}
}

km_result_t pcibus_fetch_device_classes_to_global_vars() {
	kf_uuid_t uuid;

	uuid = DM_DEVICE_CLASS_BUS_CONTROLLER;

	if (!(pcibus_bus_controller_device_class = dm_query_device_class(&uuid)))
		return KM_RESULT_DEVICE_CLASS_NOT_FOUND;

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END

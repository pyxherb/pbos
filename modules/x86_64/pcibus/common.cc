#include "common.h"
#include <pbos/ps/mutex.hh>
#include <pbos/kd/logger.h>

PBOS_EXTERN_C_BEGIN

kfxx::option_t<kfxx::map_t<uint16_t, pcibus_domain_registry_ptr>> pcibus_segment_group_id_to_domain_map;

kfxx::rbtree_t<pcibus_domain_id_t> pcibus_domain_tree;

fs::fnode_ptr pcibus_devio_pci_root_dir;

dm_bus_t *pcibus_bus_object = nullptr;

pcibus_domain_registry_t::pcibus_domain_registry_t() {}
pcibus_domain_registry_t *pcibus_domain_registry_t::alloc() {
	return kfxx::alloc_and_construct<pcibus_domain_registry_t>(kfxx::kernel_allocator());
}

static ps::mutex_t domain_id_alloc_mutex;

bool pcibus_alloc_domain_id_and_insert(pcibus_domain_registry_t *registry) {
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

		kd_println(PCIROOT_COMPONENT_NAME, "Removed directory for PCI domain: %s", name);
	}
}

PBOS_EXTERN_C_END

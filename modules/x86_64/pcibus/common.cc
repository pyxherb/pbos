#include "common.h"
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

kfxx::option_t<kfxx::map_t<uint16_t, pcibus_domain_registry_ptr>> pcibus_segment_group_id_to_domain_map;

kfxx::rbtree_t<pcibus_domain_id_t> pcibus_domain_tree;

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

PBOS_EXTERN_C_END

#include "common.h"
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

kfxx::option_t<kfxx::map_t<uint16_t, pciroot_domain_registry_ptr>> pciroot_segment_group_id_to_domain_map;

kfxx::rbtree_t<pciroot_domain_id_t> pciroot_domain_tree;

pciroot_domain_registry_t::pciroot_domain_registry_t() {}
pciroot_domain_registry_t *pciroot_domain_registry_t::alloc() {
	return kfxx::alloc_and_construct<pciroot_domain_registry_t>(kfxx::kernel_allocator());
}

static ps::mutex_t domain_id_alloc_mutex;

bool pciroot_alloc_domain_id_and_insert(pciroot_domain_registry_t *registry) {
	ps::mutex_guard g(domain_id_alloc_mutex);

	if (pciroot_domain_tree.size() == UINT16_MAX)
		return false;

	if (pciroot_domain_tree.empty()) {
		registry->rb_value = 0;
		pciroot_domain_tree.insert_unwrap(registry);
		return true;
	}

	auto end = pciroot_domain_tree.end_const();
	auto it = pciroot_domain_tree.begin_const(), last_it = it;
	for (; it != end; ++it) {
		if ((*it) > (*last_it) + 1) {
			registry->rb_value = (*last_it) + 1;
			pciroot_domain_tree.insert_unwrap(registry);
			return true;
		}
		last_it = it;
	}

	registry->rb_value = (*last_it) + 1;
	pciroot_domain_tree.insert_unwrap(registry);
	return true;
}

PBOS_EXTERN_C_END

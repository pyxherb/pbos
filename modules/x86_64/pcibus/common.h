#ifndef _PCIROOT_COMMON_H_
#define _PCIROOT_COMMON_H_

#include <pbos/dm/bus.h>
#include <pbos/kf/atomic.h>
#include <pbos/kfxx/map.hh>

PBOS_EXTERN_C_BEGIN

#define PCIROOT_COMPONENT_NAME "pcibus"

using pcibus_domain_id_t = uint32_t;

struct pcibus_domain_registry_t : public kfxx::rbtree_t<pcibus_domain_id_t>::node_t {
	size_t ref_count = 0;
	uint16_t segment_group_id;
	dm_device_t *device;

	pcibus_domain_registry_t();

	PBOS_FORCEINLINE void inc_ref() noexcept {
		kf_atomic_inc_size(&ref_count);
	}

	PBOS_FORCEINLINE void dec_ref() noexcept {
		if (!kf_atomic_dec_size(&ref_count)) {
			kfxx::destroy_and_release<pcibus_domain_registry_t>(kfxx::kernel_allocator(), this);
		}
	}

	static pcibus_domain_registry_t *alloc();
};

using pcibus_domain_registry_ptr = kfxx::rc_object_ptr<pcibus_domain_registry_t>;

extern kfxx::option_t<kfxx::map_t<uint16_t, pcibus_domain_registry_ptr>> pcibus_segment_group_id_to_domain_map;
extern kfxx::rbtree_t<pcibus_domain_id_t> pcibus_domain_tree;

/// @brief Allocate a new free domain ID.
///
/// @return Allocated domain ID, @c nullopt if failed.
bool pcibus_alloc_domain_id_and_insert(pcibus_domain_registry_t *registry);

PBOS_EXTERN_C_END

#endif

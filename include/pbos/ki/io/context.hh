#ifndef _PBOS_KI_MM_CONTEXT_HH_
#define _PBOS_KI_MM_CONTEXT_HH_

#include <pbos/kfxx/radix_map.hh>
#include <pbos/ps/mutex.hh>
#include <pbos/io/context.h>
#include <pbos/io/irq.hh>

typedef struct _io_interrupt_list_t {
	io_interrupt_t *first = nullptr;
} io_interrupt_list_t;

typedef struct _io_context_t {
	/// @brief An opaque pointer to the platform-specific interrupt table.
	void *interrupt_table = nullptr;

	kfxx::radix_map_t<io_irq_id_t, io_interrupt_list_t> registered_isrs;

	_io_context_t(kfxx::allocator_t *allocator);
} io_context_t;

extern io_context_t **ki_per_cpu_contexts;

#endif

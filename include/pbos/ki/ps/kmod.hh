#ifndef _PBOS_KI_PS_KMOD_HH_
#define _PBOS_KI_PS_KMOD_HH_

#include <pbos/ps/kmod.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/string_view.hh>
#include <pbos/kfxx/set.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _ps_kmod_section_t : public kfxx::RBTree<void *>::Node {
	size_t size = 0;
	ps_kmod_t *parent_mod = nullptr;
} ps_kmod_section_t;

typedef struct _ps_kmod_t {
	ps_kmod_t *prev = nullptr, *next = nullptr;
	kfxx::RBTree<void *> registered_sections;
	kfxx::Set<kfxx::StringView> registered_symbols;

	_ps_kmod_t(kfxx::Alloc *allocator);
	~_ps_kmod_t();
} ps_kmod_t;

extern ps_kmod_t *ki_ps_kmod_list;

km_result_t ki_ps_alloc_kmod_section(void *vaddr, size_t size, ps_kmod_t *kmod, ps_kmod_section_t **section_out);
void ki_ps_destroy_kmod_section(ps_kmod_section_t *section);

typedef struct _ki_kernel_symbol_t : public kfxx::RBTree<void *>::Node {
	char *name = nullptr;
	size_t name_len = 0;
	size_t len;
} ki_kernel_symbol_t;

extern kfxx::RBTree<void*> ki_registered_kernel_symbol_query_tree;

km_result_t ki_do_register_kernel_symbol(const char *name, size_t name_len, void *addr, size_t len, ki_kernel_symbol_t **symbol_out);
void ki_destroy_kernel_symbol(ki_kernel_symbol_t *sym);

PBOS_EXTERN_C_END

#endif

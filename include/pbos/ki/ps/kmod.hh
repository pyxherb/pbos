#ifndef _PBOS_KI_PS_KMOD_HH_
#define _PBOS_KI_PS_KMOD_HH_

#include <pbos/ps/kmod.h>
#include <pbos/kfxx/rbtree.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _ps_kmod_section_t : public kfxx::rbtree_t<void *>::node_t {
	size_t size = 0;
	ps_kmod_t *parent_mod = nullptr;
} ps_kmod_section_t;

typedef struct _ps_kmod_t {
	ps_kmod_t *prev = nullptr, *next = nullptr;
	kfxx::rbtree_t<void *> registered_sections;
} ps_kmod_t;

extern ps_kmod_t *ki_ps_kmod_list;

km_result_t ki_ps_alloc_kmod_section(void *vaddr, size_t size, ps_kmod_t *kmod, ps_kmod_section_t **section_out);
void ki_ps_destroy_kmod_section(ps_kmod_section_t *section);

PBOS_EXTERN_C_END

#endif

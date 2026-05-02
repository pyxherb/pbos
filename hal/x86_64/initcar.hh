#ifndef _HAL_X86_64_INITCAR_H_
#define _HAL_X86_64_INITCAR_H_

#include <pbos/kh/initcar.hh>
#include <pbos/fs/fs.h>

PBOS_EXTERN_C_BEGIN

struct hn_initcar_file_keeper_t : public kfxx::rbtree_t<fs_fnode_t *>::node_t {
	const char *ptr;
	size_t sz_total;

	hn_initcar_file_keeper_t(fs_fnode_t *file);
	~hn_initcar_file_keeper_t();
};

extern kfxx::rbtree_t<fs_fnode_t *> hn_initcar_file_set;

extern void *hn_initcar_ptr;
extern void *hn_initcar_paddr;
extern size_t hn_initcar_size;
extern bool hn_is_initcar_direct_mapped;

PBOS_EXTERN_C_END

#endif

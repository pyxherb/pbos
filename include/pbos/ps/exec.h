#ifndef _PBOS_PS_EXEC_H_
#define _PBOS_PS_EXEC_H_

#include <pbos/fs/file.h>
#include <pbos/se/user.h>
#include "kmod.h"

PBOS_EXTERN_C_BEGIN

typedef struct _km_binldr_ops_t {
	km_result_t (*load_exec)(ps_pcb_t *proc, fs_fcb_t *file_fp);
	km_result_t (*load_mod)(ps_pcb_t *proc, fs_fcb_t *file_fp);
	km_result_t (*load_kmod)(ps_kmod_t *kmod, fs_fcb_t *file_fp);
} km_binldr_ops_t;

typedef struct _km_init_binldr_registry_t {
	kf_uuid_t uuid;
	km_binldr_ops_t *ops;
} km_init_binldr_registry_t;

typedef struct _km_binseg_t km_binseg_t;

typedef struct _km_binproto_t km_binproto_t;

km_result_t ps_exec(
	ps_proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	ps_proc_id_t *pid_out);
km_result_t ps_register_binldr(kf_uuid_t *uuid, km_binldr_ops_t *binldr);

///
/// @brief Register a cached read-only data page. The page will be referenced 1 time after successfully registered.
///
/// @param paddr Physical address of the page.
/// @param vaddr Virtual address of the mapped page, used for hashing the content.
/// @return Result indicating if the operation was performed successfully or any error occured.
///
km_result_t ps_register_cached_ro_page(void *paddr, void *allocated_cmp_vpage, void *vaddr);
km_result_t ps_unregister_cached_ro_page(void *paddr);
///
/// @brief Fetch a cached read-only data page.
///
/// @param vaddr Virtual address to the page to be queried.
/// @param allocated_cmp_vpage An virtual page for comparison which should be provided and freed by the caller.
/// @param paddr_out Address where the physical address of the page with the same content if there is.
/// @return @c KM_RESULT_OK if found, @c KM_RESULT_NOT_FOUND if not found.
///
km_result_t ps_fetch_cached_ro_page(void *vaddr, void *allocated_cmp_vpage, void **paddr_out);
void ps_ref_cached_ro_page(void *paddr);
void ps_unref_cached_ro_page(void *paddr);

km_result_t ps_register_binproto(fs_fcb_t *fcb, km_binproto_t **proto_out);
km_binproto_t *ps_find_binproto(fs_fcb_t *fcb);
void ps_unregister_binproto(km_binproto_t *proto);
///
/// @brief Add a segment to a binproto.
///
/// @param proto binproto to be operated.
/// @param vaddr_base Base virtual address to the segment, must be page-aligned, or @c KM_RESULT_INVALID_ARGS will be returned.
/// @param size Size of the segment.
/// @param pgaccess Page access to be applied to the segment.
/// @return KM_RESULT_INVALID_ARGS If the arguments are invalid.
///
km_result_t ps_add_segment_to_binproto(km_binproto_t *proto, void *vaddr_base, size_t size, mm_pgaccess_t pgaccess, km_binseg_t **seg_out);
void *km_ps_paddr_in_binseg(km_binseg_t *seg, void *vaddr);

PBOS_EXTERN_C_END

#endif

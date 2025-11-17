#ifndef _PBOS_KM_EXEC_H_
#define _PBOS_KM_EXEC_H_

#include <pbos/fs/file.h>
#include <pbos/se/user.h>
#include "proc.h"

PBOS_EXTERN_C_BEGIN

typedef struct _km_binldr_t {
	km_result_t (*load_exec)(ps_pcb_t *proc, fs_fcb_t *file_fp);
	km_result_t (*load_mod)(ps_pcb_t *proc, fs_fcb_t *file_fp);
} km_binldr_t;

typedef struct _km_init_binldr_registry_t {
	kf_uuid_t uuid;
	km_binldr_t *binldr;
} km_init_binldr_registry_t;

typedef struct _km_binseg_t km_binseg_t;

typedef struct _km_binproto_t km_binproto_t;

km_result_t km_exec(
	proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	proc_id_t *pid_out);
km_result_t km_exec_init(
	proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	proc_id_t *pid_out);
km_result_t km_register_binldr(kf_uuid_t *uuid, km_binldr_t *binldr);

km_result_t km_register_binproto(fs_fcb_t *fcb, km_binproto_t **proto_out);
km_binproto_t *km_find_binproto(fs_fcb_t *fcb);
void km_unregister_binproto(km_binproto_t *proto);
///
/// @brief Add a segment to a binproto.
///
/// @param proto binproto to be operated.
/// @param vaddr_base Base virtual address to the segment, must be page-aligned, or `KM_RESULT_INVALID_ARGS` will be returned.
/// @param size Size of the segment.
/// @param pgaccess Page access to be applied to the segment.
/// @return KM_RESULT_INVALID_ARGS If the arguments are invalid.
///
km_result_t km_add_segment_to_binproto(km_binproto_t *proto, void *vaddr_base, size_t size, mm_pgaccess_t pgaccess, km_binseg_t **seg_out);
void *km_get_paddr_in_binseg(km_binseg_t *seg, void *vaddr);

PBOS_EXTERN_C_END

#endif

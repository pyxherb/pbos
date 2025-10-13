#ifndef _PBOS_KM_EXEC_H_
#define _PBOS_KM_EXEC_H_

#include <pbos/fs/file.h>
#include <pbos/se/user.h>
#include "proc.h"

PB_EXTERN_C_BEGIN

typedef struct _km_binldr_t {
	uuid_t uuid;
	km_result_t (*load_exec)(ps_pcb_t *proc, fs_fcb_t *file_fp);
	km_result_t (*load_mod)(ps_pcb_t *proc, fs_fcb_t *file_fp);
} km_binldr_t;

km_result_t km_exec(
	proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	proc_id_t *pid_out);
km_result_t km_register_binldr(km_binldr_t *binldr);

PB_EXTERN_C_END

#endif

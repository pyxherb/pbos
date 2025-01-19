#include "pbcore.h"
#include <pbos/fs/file.h>
#include <pbos/km/exec.h>

void *sysent_exit(uint32_t exitcode) {

}

int sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_uhandle_t *uhandle_out) {
	km_result_t result;
	ps_pcb_t *pcb = ps_get_cur_proc();

	if(mm_is_user_access_violated(pcb->mm_context, path, path_len))
		return KM_RESULT_ACCESS_VIOLATION;
	if(mm_is_user_access_violated(pcb->mm_context, uhandle_out, sizeof(ps_uhandle_t)))
		return KM_RESULT_ACCESS_VIOLATION;

	fs_fcontext_t *fcontext;
	if(KM_FAILED(result = fs_open(pcb->cur_dir, path, path_len, &fcontext)))
		return result;

	om_handle_t handle;
	if(KM_FAILED(om_create_handle(&fcontext->object_header, handle))) {
		fs_close(fcontext);
		return result;
	}

	if(KM_FAILED(result = ps_create_uhandle(pcb, handle, uhandle_out))) {
		om_close_handle(handle);
		fs_close(fcontext);
		return result;
	}

	return KM_RESULT_OK;
}

void sysent_close(int fd, uint32_t flags) {

}

void *sysent_read(int fd, void *buf, uint32_t size) {

}

void *sysent_write(int fd, const void *buf, uint32_t size) {

}

km_result_t sysent_exec_child(
	ps_uhandle_t file_handle,
	ps_uhandle_t cwd_handle,
	const char *args,
	size_t args_len,
	proc_id_t *proc_id_out
) {
	ps_pcb_t *pcb = ps_get_cur_proc();
	km_result_t result;

	if(mm_is_user_access_violated(pcb->mm_context, args, args_len))
		return KM_RESULT_ACCESS_VIOLATION;

	if(mm_is_user_access_violated(pcb->mm_context, proc_id_out, sizeof(proc_id_t)))
		return KM_RESULT_ACCESS_VIOLATION;

	om_handle_t file_khandle = ps_lookup_uhandle(pcb, file_handle);
	if(file_khandle == OM_INVALID_HANDLE)
		return KM_RESULT_INVALID_ARGS;

	om_handle_t cwd_khandle = ps_lookup_uhandle(pcb, cwd_handle);
	if(cwd_handle == OM_INVALID_HANDLE)
		return KM_RESULT_INVALID_ARGS;

	om_object_t *extracted_file;
	if(KM_FAILED(result = om_deref_handle(file_khandle, &extracted_file)))
		return result;

	return km_exec(pcb->proc_id, 0, PB_CONTAINER_OF(fs_fcontext_t, object_header, extracted_file), proc_id_out);
}

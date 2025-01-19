#include "pbcore.h"
#include <pbos/fs/file.h>
#include <pbos/km/exec.h>

void *sysent_exit(uint32_t exitcode) {
}

km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_ufd_t *ufd_out) {
	km_result_t result;
	ps_pcb_t *pcb = ps_get_cur_proc();

	if (mm_is_user_access_violated(pcb->mm_context, path, path_len))
		return KM_RESULT_ACCESS_VIOLATION;
	if (mm_is_user_access_violated(pcb->mm_context, ufd_out, sizeof(*ufd_out)))
		return KM_RESULT_ACCESS_VIOLATION;

	ps_ufd_t fd = ps_alloc_fd(pcb);
	if(fd < 0)
		return KM_RESULT_NO_SLOT;

	fs_fcontext_t *fcontext;
	if (KM_FAILED(result = fs_open(pcb->cur_dir, path, path_len, &fcontext))) {
		return result;
	}

	ps_ufcontext_t *ufcontext;
	if(!(ufcontext = ps_alloc_ufcontext(pcb, fcontext, fd))) {
		fs_close(fcontext);
		return KM_RESULT_NO_MEM;
	}

	*fcontext_out = fcontext;

	return KM_RESULT_OK;
}

km_result_t sysent_close(ps_ufd_t ufd, uint32_t flags) {
	km_result_t result;
	ps_pcb_t *pcb = ps_get_cur_proc();

	ps_ufcontext_t *ufcontext = ps_lookup_ufcontext(pcb, ufd);

	if(!ufcontext)
		return KM_RESULT_INVALID_ARGS;

	ps_remove_ufcontext(pcb, ufcontext);

	return KM_RESULT_OK;
}

void *sysent_read(ps_ufd_t ufd, void *buf, uint32_t size) {
}

void *sysent_write(ps_ufd_t ufd, const void *buf, uint32_t size) {
}

km_result_t sysent_exec_child(
	ps_ufd_t file_ufd,
	ps_ufd_t cwd_ufd,
	const char *args,
	size_t args_len,
	proc_id_t *proc_id_out) {
	ps_pcb_t *pcb = ps_get_cur_proc();
	km_result_t result;

	if (mm_is_user_access_violated(pcb->mm_context, args, args_len))
		return KM_RESULT_ACCESS_VIOLATION;

	if (mm_is_user_access_violated(pcb->mm_context, proc_id_out, sizeof(proc_id_t)))
		return KM_RESULT_ACCESS_VIOLATION;

	ps_ufcontext_t *ufcontext = ps_lookup_ufcontext(pcb, file_ufd);

	if(!ufcontext)
		return KM_RESULT_INVALID_ARGS;

	return km_exec(pcb->proc_id, 0, ufcontext->kernel_fcontext, proc_id_out);
}

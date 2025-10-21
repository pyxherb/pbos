#include "pbcore.h"
#include <pbos/fs/file.h>
#include <pbos/km/exec.h>
#include "pbos/hal/irq.hh"

PBOS_EXTERN_C_BEGIN

void sysent_exit(int exitcode) {
}

km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_ufd_t *ufd_out) {
	km_result_t result;
	ps_euid_t euid = ps_get_cur_euid();
	ps_pcb_t *pcb = ps_get_cur_proc();

	if (mm_probe_user_space(pcb->mm_context, path, path_len))
		return KM_RESULT_ACCESS_VIOLATION;
	if (mm_probe_user_space(pcb->mm_context, ufd_out, sizeof(*ufd_out)))
		return KM_RESULT_ACCESS_VIOLATION;

	ps_ufd_t fd = ps_alloc_fd(pcb);
	if (fd < 0)
		return KM_RESULT_NO_SLOT;

	fs_fcb_t *fcb;
	{
		if (KM_FAILED(result = fs_open(pcb->cur_dir, path, path_len, &fcb))) {
			return result;
		};
	}

	ps_ufcb_t *ufcb;
	if (!(ufcb = ps_alloc_ufcb(pcb, fcb, fd))) {
		fs_close(fcb);
		return KM_RESULT_NO_MEM;
	}

	*ufd_out = fd;

	return KM_RESULT_OK;
}

km_result_t sysent_close(ps_ufd_t ufd, uint32_t flags) {
	km_result_t result;
	ps_pcb_t *pcb = ps_get_cur_proc();

	ps_ufcb_t *ufcb = ps_lookup_ufcb(pcb, ufd);

	if (!ufcb)
		return KM_RESULT_INVALID_ARGS;

	ps_remove_ufcb(pcb, ufcb);

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

	if (mm_probe_user_space(pcb->mm_context, args, args_len))
		return KM_RESULT_ACCESS_VIOLATION;

	if (mm_probe_user_space(pcb->mm_context, proc_id_out, sizeof(proc_id_t)))
		return KM_RESULT_ACCESS_VIOLATION;

	ps_ufcb_t *ufcb = ps_lookup_ufcb(pcb, file_ufd);

	if (!ufcb)
		return KM_RESULT_INVALID_ARGS;

	return km_exec(pcb->rb_value, 0, ufcb->kernel_fcb, proc_id_out);
}

PBOS_EXTERN_C_END

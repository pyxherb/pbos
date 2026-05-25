#include <pbos/fs/file.h>
#include <pbos/ps/exec.h>
#include <pbos/ps/proc.h>
#include <pbos/syscall/pbcore.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

km_result_t sysent_exit(int exitcode) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_ufd_t *ufd_out) {
	km_result_t result;

	ps_cpuid_t cpuid = ps_get_cur_cpuid();
	ps_pcb_t *pcb = ps_get_cur_proc();
	mm_context_t *mm_context = ps_mm_context_of(pcb);

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, (void *)path, path_len, MM_PAGE_READ));
	kfxx::deferred unlock_path_guard([mm_context, path, path_len]() noexcept {
		mm_unlock_pages(mm_context, (void *)path, path_len);
	});

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, ufd_out, sizeof(*ufd_out), MM_PAGE_WRITE));
	kfxx::deferred unlock_ufd_out_guard([mm_context, ufd_out]() noexcept {
		mm_unlock_pages(mm_context, ufd_out, sizeof(*ufd_out));
	});

	ps_ufd_t fd = ps_alloc_fd(pcb);
	if (fd < 0)
		return KM_RESULT_NO_SLOT;

	fs_fcb_t *fcb;
	{
		size_t len_cwd;
		if (KM_FAILED(result = fs_open(ps_get_cwd(pcb), path, path_len, &fcb))) {
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

km_result_t sysent_read(ps_ufd_t ufd, void *buf, uint32_t size, size_t off, size_t *bytes_read_out) {
	ps_pcb_t *pcb = ps_get_cur_proc();
	mm_context_t *mm_context = ps_mm_context_of(pcb);

	// TODO: Unlock the pages.

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, buf, size, MM_PAGE_READ));
	kfxx::deferred unlock_buf_guard([mm_context, buf, size]() noexcept {
		mm_unlock_pages(mm_context, buf, size);
	});

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, bytes_read_out, sizeof(size_t), MM_PAGE_WRITE));
	kfxx::deferred unlock_byte_read_out_guard([mm_context, bytes_read_out]() noexcept {
		mm_unlock_pages(mm_context, bytes_read_out, sizeof(size_t));
	});

	ps_ufcb_t *ufcb = ps_lookup_ufcb(pcb, ufd);

	if (!ufcb)
		return KM_RESULT_INVALID_ARGS;

	return fs_read(ps_kfcb_of_ufcb(ufcb), buf, size, off, bytes_read_out);
}

km_result_t sysent_write(ps_ufd_t ufd, const void *buf, uint32_t size) {
	ps_pcb_t *pcb = ps_get_cur_proc();
	mm_context_t *mm_context = ps_mm_context_of(pcb);

	// TODO: Unlock the pages.

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, (void *)buf, size, MM_PAGE_READ));
}

km_result_t sysent_exec_child(
	ps_ufd_t file_ufd,
	ps_ufd_t cwd_ufd,
	const char *args,
	size_t args_len,
	ps_proc_id_t *proc_id_out) {
	ps_pcb_t *pcb = ps_get_cur_proc();
	mm_context_t *mm_context = ps_mm_context_of(pcb);
	km_result_t result;

	// TODO: Unlock the pages.

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, (void *)args, args_len, MM_PAGE_READ));
	kfxx::deferred unlock_args_guard([mm_context, args, args_len]() noexcept {
		mm_unlock_pages(mm_context, (void *)args, args_len);
	});

	KM_RETURN_IF_FAILED(mm_probe_and_lock_user_pages(mm_context, proc_id_out, sizeof(ps_proc_id_t), MM_PAGE_WRITE));
	kfxx::deferred unlock_proc_id_out_guard([mm_context, proc_id_out]() noexcept {
		mm_unlock_pages(mm_context, proc_id_out, sizeof(ps_proc_id_t));
	});

	ps_ufcb_t *ufcb = ps_lookup_ufcb(pcb, file_ufd);

	if (!ufcb)
		return KM_RESULT_INVALID_ARGS;

	return ps_exec(ps_pid_of(pcb), 0, ps_kfcb_of_ufcb(ufcb), proc_id_out);
}

km_result_t sysent_fork() {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

PBOS_EXTERN_C_END

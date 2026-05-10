#include <pbos/hal/init.h>
#include <pbos/hal/irq.h>
#include <pbos/km/logger.h>
#include <pbos/km/panic.h>
#include <pbos/mm/mm.h>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/kh/mp/init.hh>
#include <pbos/ki/exec/exec.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/km/proc.hh>

// Because the operating system will never exit normally,
// we just designed a dummy procedure to register the destructors.
PBOS_EXTERN_C int atexit(void (*func)(void)) {
	return 0;
}

PBOS_EXTERN_C PBOS_NORETURN void kernel_main() {
	km_result_t result;

	hal_init();
	// kh_irq_init();

	if (kh_acpi_is_available()) {
		kh_acpi_init();
	} else {
		if (kh_acpi_is_required())
			km_panic("PbOS requires ACPI to start up.");
	}

	kh_mp_init_topology();

	kh_mp_alloc_platform_resources();

	mp_alloc_resources();

	kh_irq_init();

	mp_main_cpu_init();

	fs_init();
	ps_init();

	kh_initcar_init();

	auto init_close_fail_hook([&result](fs_fcb_t *fcb, km_result_t result_in) noexcept {
		result = result_in;
	});
	{
		fs::fcb_ptr_t init_fp(nullptr, std::move(init_close_fail_hook));
		if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_fp)))
			km_panic("Error opening the init executable");

		ps_proc_id_t pid;

		if (KM_FAILED(result = km_exec(0, 0, init_fp.get(), &pid)))
			km_panic("Error starting the init process");
	}
	if (KM_FAILED(result))
		km_panic("Error closing the init FCB");

	kh_enter_sched(0);
}

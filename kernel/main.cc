#include <pbos/hal/init.h>
#include <pbos/hal/irq.h>
#include <pbos/km/logger.h>
#include <pbos/mm/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kn/fs/fs.hh>
#include <pbos/kn/fs/initcar.hh>
// #include <pbos/kn/km/exec.hh>
#include <pbos/kn/km/mp.h>
#include <pbos/kn/km/proc.hh>
#include <pbos/kh/acpi/misc.hh>

// Because the operating system will never exit normally,
// we just designed a dummy procedure to register the destructors.
PBOS_EXTERN_C int atexit(void (*func)(void)) {
	return 0;
}

PBOS_EXTERN_C PBOS_NORETURN void kernel_main() {
	km_result_t result;

	hal_init();
	// hal_irq_init();

	fs_init();
	ps_init();

	if(kh_acpi_is_available()) {
		kh_acpi_init();
	} else {
		if(kh_acpi_is_required())
			km_panic("PbOS requires ACPI to start up.");
	}

	mp_init();

	asm volatile("hlt");

	/*initcar_init();

	fs_fcb_t *init_fp;
	if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_fp)))
		km_panic("Error opening the init executable");

	ps_proc_id_t pid;

	if (KM_FAILED(result = km_exec(0, 0, init_fp, &pid)))
		km_panic("Error starting the init process");

	kn_enter_sched(0);*/
}

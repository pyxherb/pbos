#include <hal/i386/debug.h>
#include <hal/i386/initcar.h>
#include <pbos/hal/init.h>
#include <pbos/hal/irq.h>
#include <pbos/km/logger.h>
#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kn/fs/fs.h>
#include <pbos/kn/fs/initcar.h>
#include <pbos/kn/km/exec.h>
#include <pbos/kn/km/objmgr.h>

// Because the operating system will never exit normally,
// we just using a dummy procedure to register the destructors.
extern "C" int atexit(void (*func)(void)) {
	return 0;
}

extern "C" PB_NORETURN void _start() {
	km_result_t result;

	hal_init();
	// irq_init();

	om_init();
	fs_init();
	ps_init();

	initcar_init();

	fs_fcb_t *init_fp;
	if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_fp)))
		km_panic("Error opening the init executable");

	proc_id_t pid;

	if (KM_FAILED(result = km_exec(0, 0, init_fp, &pid)))
		km_panic("Error starting the init process");

	kn_enter_sched(0);
}

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
#include <string.h>

PB_NORETURN void _start() {
	km_result_t result;

	hal_init();
	// irq_init();

	om_init();
	fs_init();
	ps_init();

	initcar_init();

	om_handle_t init_handle;
	if (KM_FAILED(fs_open("/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_handle)))
		km_panic("Error opening the init executable");

	proc_id_t pid;

	if (KM_FAILED(result = km_exec(0, 0, init_handle, &pid)))
		km_panic("Error starting the init process");

	initcar_deinit();

	__asm__ __volatile__("hlt");
}

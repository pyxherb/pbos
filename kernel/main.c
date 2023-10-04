#include <hal/i386/debug.h>
#include <hal/i386/initcar.h>
#include <oicos/hal/init.h>
#include <oicos/hal/irq.h>
#include <oicos/km/logger.h>
#include <oicos/km/mm.h>
#include <oicos/km/panic.h>
#include <oicos/kn/fs/fs.h>
#include <oicos/kn/fs/initcar.h>
#include <oicos/kn/km/exec.h>
#include <oicos/kn/km/objmgr.h>
#include <string.h>

void __noreturn _start() {
	km_result_t result;

	hal_init();
	// irq_init();

	om_init();

	fs_init();

	initcar_init();

	kn_init_exec();

	om_handle_t init_handle;
	if (KM_FAILED(fs_open("/initcar/insinit", sizeof("/initcar/insinit") - 1, &init_handle)))
		km_panic("Error opening the init executable");

	proc_id_t pid;

	if(KM_FAILED(result=km_exec(0, 0, init_handle, &pid)))
		km_panic("Error starting the init process");

	initcar_deinit();

	__asm__ __volatile__("hlt");
}

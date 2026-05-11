#include <pbos/hal/init.h>
#include <pbos/hal/irq.h>
#include <pbos/km/logger.h>
#include <pbos/km/panic.h>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/kh/mm/init.hh>
#include <pbos/kh/mp/init.hh>
#include <pbos/ki/exec/exec.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

const ki_syment_t KI_EXPORTED_SYMBOLS_BEGIN[0] = {}, KI_EXPORTED_SYMBOLS_END[0] = {};

typedef void (*ki_ctor_t)();
typedef void (*ki_dtor_t)();

alignas(ki_ctor_t) ki_ctor_t KI_CTORS_BEGIN[0] = {};
alignas(ki_ctor_t) ki_ctor_t KI_CTORS_END[0] = {};

alignas(ki_dtor_t) ki_dtor_t KI_DTORS_BEGIN[0] = {};
alignas(ki_dtor_t) ki_dtor_t KI_DTORS_END[0] = {};

extern "C" void __cxa_pure_virtual() {
	km_panic("Attempting to call a pure virtual function!");
}

// Because the operating system will never exit normally,
// we just designed a dummy procedure to register the destructors.
int atexit(void (*func)(void)) {
	return 0;
}

void ki_call_ctors() {
	const size_t n_ctors = KI_CTORS_END - KI_CTORS_BEGIN;
	for (size_t i = 0; i < n_ctors; ++i) {
		KI_CTORS_BEGIN[i]();
	}
}

PBOS_NORETURN void kernel_main() {
	km_result_t result;

	ki_call_ctors();

	hal_init();

	klog_set_logger(klog_get_default_logger());

	kh_mm_init();

	// kh_irq_init();

	ki_mm_init_global_allocator();

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

	for (const ki_syment_t *i = KI_EXPORTED_SYMBOLS_BEGIN; i < KI_EXPORTED_SYMBOLS_END; ++i) {
		kd_printf("Found image symbol: %s\n", i->name);
	}

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

PBOS_EXTERN_C_END

#include <pbos/hal/init.h>
#include <pbos/hal/irq.h>
#include <pbos/kd/logger.h>
#include <pbos/km/panic.h>
#include <pbos/kfxx/map.hh>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/kh/mm/init.hh>
#include <pbos/kh/mp/init.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/km/proc.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/ps/exec.hh>
#include <pbos/ki/ps/kmod.hh>

PBOS_EXTERN_C_BEGIN

extern const Elf64_Sym KI_EXPORTED_SYMBOLS_BEGIN[], KI_EXPORTED_SYMBOLS_END[];
extern const char KI_EXPORTED_SYMBOL_NAMES_BEGIN[], KI_EXPORTED_SYMBOL_NAMES_END[];

typedef void (*ki_ctor_t)();
typedef void (*ki_dtor_t)();

extern ki_ctor_t KI_CTORS_BEGIN[] = {};
extern ki_ctor_t KI_CTORS_END[] = {};

extern ki_dtor_t KI_DTORS_BEGIN[] = {};
extern ki_dtor_t KI_DTORS_END[] = {};

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

	kd_set_logger(kd_default_logger());

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

	ki_mp_alloc_resources();

	kh_irq_init();

	mp_main_cpu_init();

	ki_fs_init();
	ki_ps_init();

	/*for (const ki_syment_t *i = KI_EXPORTED_SYMBOLS_BEGIN; i < KI_EXPORTED_SYMBOLS_END; ++i) {
		dbg_printf("Found image symbol: %s\n", i->name);
	}*/

	kd_println("kernel", "Scanning for kernel symbols...");
	{
		for (const Elf64_Sym *i = KI_EXPORTED_SYMBOLS_BEGIN; i != KI_EXPORTED_SYMBOLS_END; ++i) {
			uint8_t visibility = ELF64_ST_VISIBILITY(i->st_other);
			kfxx::StringView name = kfxx::StringView(KI_EXPORTED_SYMBOL_NAMES_BEGIN + i->st_name);

			if ((visibility == STV_DEFAULT) && i->st_value) {
				kd_println("kernel", "Found public symbol: %s -> %p", name.data(), (void *)i->st_value);
				// TODO: Handle value when ASLR enabled...
				if (KM_FAILED(result = ki_do_register_kernel_symbol(name.data(), name.size(), (void *)i->st_value, i->st_size, nullptr))) {
					switch (result) {
						case KM_RESULT_NO_MEM:
							km_panic("Error registering kernel symbols because of out of memory");
						default:
							km_panic("Error registering kernel symbol %s, result = %u", name.data(), result);
					}
				}
			} else {
				kd_println("kernel", "Skipped symbol: %s -> %p", name.data(), (void *)i->st_value);
			}
		}
	}
	kd_println("kernel", "Symbol scanning completed");

	kh_initcar_init();

	auto init_close_fail_hook([&result](fs_fcb_t *fcb, km_result_t result_in) noexcept {
		result = result_in;
	});
	{
		fs::FcbPtr init_fp(nullptr, std::move(init_close_fail_hook));
		if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_fp)))
			km_panic("Error opening the init executable");

		ps_proc_id_t pid;

		if (KM_FAILED(result = ps_exec(0, 0, init_fp.get(), &pid)))
			km_panic("Error starting the init process");
	}
	if (KM_FAILED(result))
		km_panic("Error closing the init FCB");

	kh_enter_sched(0);
}

PBOS_EXTERN_C_END

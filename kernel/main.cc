#include <pbos/hal/init.h>
#include <pbos/kd/logger.h>
#include <pbos/km/panic.h>
#include <pbos/kfxx/map.hh>
#include <pbos/kh/acpi/misc.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/kh/io/irq.hh>
#include <pbos/kh/mm/init.hh>
#include <pbos/kh/mp/init.hh>
#include <pbos/ki/dm/devcls.hh>
#include <pbos/ki/fs/devio.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/kasan/impl.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/ps/exec.hh>
#include <pbos/ki/ps/kmod.hh>
#include <pbos/ki/ps/proc.hh>

PBOS_EXTERN_C_BEGIN

extern const Elf64_Sym KI_EXPORTED_SYMBOLS_BEGIN[], KI_EXPORTED_SYMBOLS_END[];
extern const char KI_EXPORTED_SYMBOL_NAMES_BEGIN[], KI_EXPORTED_SYMBOL_NAMES_END[];

typedef void (*ki_ctor_t)();
typedef void (*ki_dtor_t)();

extern ki_ctor_t KI_CTORS_BEGIN[];
extern ki_ctor_t KI_CTORS_END[];

extern ki_dtor_t KI_DTORS_BEGIN[];
extern ki_dtor_t KI_DTORS_END[];

PBOS_NO_ASAN PBOS_API void __cxa_pure_virtual() {
	km_panic("Attempting to call a pure virtual function!");
}

// Because the operating system will never exit normally,
// we just designed a dummy procedure to register the destructors.
PBOS_NO_ASAN int atexit(void (*func)(void)) {
	return 0;
}

PBOS_NO_ASAN void ki_call_ctors() {
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

	ki_init_page_alloc_counter();

	// kh_irq_init();

	ki_mm_init_global_allocator();

	if (kh_acpi_is_supported()) {
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

	kh_init_irq_for_cpu(0);

	ki_fs_init();
	ki_ps_init();

	dbg_println("kernel", "Scanning for kernel symbols...");
	const size_t exported_symbols_section_size = KI_EXPORTED_SYMBOL_NAMES_END - KI_EXPORTED_SYMBOL_NAMES_BEGIN;
	{
		for (const Elf64_Sym *i = KI_EXPORTED_SYMBOLS_BEGIN; i != KI_EXPORTED_SYMBOLS_END; ++i) {
			uint8_t visibility = ELF64_ST_VISIBILITY(i->st_other);

			if ((visibility == STV_DEFAULT) && i->st_value) {
				if (i->st_name >= exported_symbols_section_size)
					km_panic("Kernel symbol table damaged");

				kfxx::string_view name = kfxx::string_view(KI_EXPORTED_SYMBOL_NAMES_BEGIN + i->st_name);
				// dbg_println("kernel", "Found public symbol: %s -> %p", name.data(), (void *)i->st_value);
				// TODO: Handle value when ASLR enabled...

				if (name.size()) {
					if (KM_FAILED(result = ki_do_register_kernel_symbol(name.data(), name.size(), (void *)i->st_value, i->st_size, nullptr))) {
						switch (result) {
							case KM_RESULT_NO_MEM:
								km_panic("Error registering kernel symbols because of out of memory");
							default:
								km_panic("Error registering kernel symbol %s, result = %u", name.data(), result);
						}
					}
				}
			}
		}
	}
	dbg_println("kernel", "Symbol scanning completed");

	ki_initcar_init();
	ki_devio_init();

	ki_register_device_classes();

	auto kmod_close_fail_hook([&result](fs_fcb_t *fcb, km_result_t result_in) noexcept {
		result = result_in;
	});
	{
		fs::fcb_ptr kmod_fp(nullptr, std::move(kmod_close_fail_hook));
		if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pcibus.kx", sizeof("/initcar/pcibus.kx") - 1, &kmod_fp, FS_OPEN_READ)))
			km_panic("Error opening the initial kernel modules");

		ps_kmod_t *kmod = nullptr;
		if (KM_FAILED(result = ps_load_kmod(kmod_fp.get(), &kmod)))
			km_panic("Error loading the initial kernel modules");
	}
	if (KM_FAILED(result))
		km_panic("Error closing the kernel module FCB");

	auto init_close_fail_hook([&result](fs_fcb_t *fcb, km_result_t result_in) noexcept {
		result = result_in;
	});
	{
		fs::fcb_ptr init_fp(nullptr, std::move(init_close_fail_hook));
		if (KM_FAILED(fs_open(fs_abs_root_dir, "/initcar/pbinit", sizeof("/initcar/pbinit") - 1, &init_fp, FS_OPEN_READ)))
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

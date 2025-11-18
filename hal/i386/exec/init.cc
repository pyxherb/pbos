#include <arch/i386/kargs.h>
#include <pbos/km/panic.h>
#include <string.h>
#include <pbos/kn/km/exec.hh>

PBOS_EXTERN_C_BEGIN

extern km_binldr_t kn_binldr_elf;

km_init_binldr_registry_t kn_builtin_binldrs[] = {
	{ .uuid = UUID(0a2c4e6a, 8c0e, 2a4c, 6e8a, 0c2e4a6c8e0a),
		.binldr = &kn_binldr_elf },
	{ .binldr = nullptr }
};

PBOS_EXTERN_C_END

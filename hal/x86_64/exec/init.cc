#include <arch/x86_64/kargs.h>
#include <pbos/km/panic.h>
#include <string.h>
#include <pbos/ki/ps/exec.hh>

PBOS_EXTERN_C_BEGIN

extern km_binldr_ops_t ki_binldr_elf;

km_init_binldr_registry_t ki_builtin_binldrs[] = {
	{ .uuid = KF_UUID(0a2c4e6a, 8c0e, 2a4c, 6e8a, 0c2e4a6c8e0a),
		.ops = &ki_binldr_elf },
	{ .ops = nullptr }
};

PBOS_EXTERN_C_END

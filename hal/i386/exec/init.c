#include <arch/i386/kargs.h>
#include <hal/i386/initcar.h>
#include <oicos/km/panic.h>
#include <oicos/kn/km/exec.h>
#include <string.h>

extern km_binldr_t kn_binldr_elf;

km_binldr_t* kn_builtin_binldrs[] = { &kn_binldr_elf, NULL };

void kn_init_exec() {
	kf_rbtree_init(
		&kn_registered_binldrs,
		kn_binldr_reg_nodecmp,
		kn_binldr_reg_keycmp,
		kn_binldr_reg_nodefree);

	for (km_binldr_t **i = kn_builtin_binldrs; *i; ++i) {
		km_register_binldr(*i);
	}
}

#include <arch/i386/kargs.h>
#include <hal/i386/initcar.h>
#include <pbos/km/panic.h>
#include <pbos/kn/km/exec.h>
#include <string.h>

extern km_binldr_t kn_binldr_elf;

km_binldr_t* kn_builtin_binldrs[] = { &kn_binldr_elf, NULL };

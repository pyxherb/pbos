#include <pbos/km/exec.h>
#include <pbos/kn/fs/fs.hh>
#include <pbos/kn/fs/initcar.hh>

#define INIT_PATH "/initcar/init"

constexpr kfxx::string_view sv = kfxx::string_view("/initcar/init");

PBOS_EXTERN_C_BEGIN

void kn_load_init() {
	fs::fcb_ptr_t init_fp;
	if (KM_FAILED(fs_open(fs_abs_root_dir, sv.data(), sv.size(), init_fp.get_addr())))
		km_panic("Error loading init from path: %s", INIT_PATH);
}

PBOS_EXTERN_C_END

#include <pbos/exec/exec.h>
#include <pbos/kn/fs/fs.hh>
#include <pbos/fs/file.hh>

constexpr kfxx::string_view INIT_PATH = kfxx::string_view("/initcar/init");

PBOS_EXTERN_C_BEGIN

void ki_load_init() {
	fs::fcb_ptr_t init_fp;
	if (KM_FAILED(fs_open(fs_abs_root_dir, INIT_PATH.data(), INIT_PATH.size(), init_fp.get_addr())))
		km_panic("Error loading init from path: %s", INIT_PATH.data());
}

PBOS_EXTERN_C_END

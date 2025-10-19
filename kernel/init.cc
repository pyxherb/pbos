#include <pbos/km/exec.h>
#include <pbos/kn/fs/initcar.h>
#include <pbos/kn/fs/fs.hh>

#define INIT_PATH "/initcar/init"

PBOS_EXTERN_C_BEGIN

void kn_load_init() {
	fs_fcb_t *init_fp;
	if(KM_FAILED(fs_open(fs_abs_root_dir, INIT_PATH, sizeof(INIT_PATH)-1,&init_fp)))
		km_panic("Error loading init from path: %s", INIT_PATH);
	fs_close(init_fp);
}

PBOS_EXTERN_C_END

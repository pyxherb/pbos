#include <pbos/km/exec.h>
#include <pbos/kn/fs/initcar.h>

#define INIT_PATH "/initcar/init"

void kn_load_init() {
	fs_fcontext_t *init_fp;
	if(KM_FAILED(fs_open(INIT_PATH, sizeof(INIT_PATH)-1,&init_fp)))
		km_panic("Error loading init from path: " INIT_PATH);
	fs_close(init_fp);
}

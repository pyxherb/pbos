#include <pbos/km/exec.h>
#include <pbos/kn/fs/initcar.h>

#define INIT_PATH "/initcar/init"

void kn_load_init() {
	om_handle_t init_handle;
	if(KM_FAILED(fs_open(INIT_PATH, sizeof(INIT_PATH)-1,&init_handle)))
		km_panic("Error loading init from path: " INIT_PATH);
	fs_close_file(init_handle);
}

#include <pbos/kd/logger.h>
#include <pbos/dm/bus.h>

km_result_t module_init() {
	dbg_printf("PCI root initialization\n");
	return KM_RESULT_OK;
}

void module_deinit() {
}

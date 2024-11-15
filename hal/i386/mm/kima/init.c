#include "init.h"
#include <pbos/km/logger.h>

void kima_init() {
	kf_rbtree_init(
		&kima_vpgdesc_query_tree,
		kima_vpgdesc_nodecmp,
		kima_vpgdesc_nodefree);
	kf_rbtree_init(
		&kima_ublk_query_tree,
		kima_ublk_nodecmp,
		kima_ublk_nodefree);

	kdprintf("Initialized KIMA\n");
}

void kima_deinit() {

}

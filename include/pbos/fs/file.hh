#ifndef _PBOS_FS_FILE_HH_
#define _PBOS_FS_FILE_HH_

#include "file.h"
#include <pbos/kfxx/rcobj.hh>

namespace fs {
	struct fnode_inc_ref {
		void operator()(fs_fnode_t *ptr) noexcept {
			fs_inc_fnode_ref(ptr);
		}
	};

	struct fnode_dec_ref {
		void operator()(fs_fnode_t *ptr) noexcept {
			fs_dec_fnode_ref(ptr);
		}
	};

	using fnode_ptr_t = kfxx::custom_rc_ptr_t<fs_fnode_t, fnode_inc_ref, fnode_dec_ref>;

	struct fcb_inc_ref {
		void operator()(fs_fcb_t *ptr) noexcept {
			fs_inc_fcb_ref(ptr);
		}
	};

	struct fcb_dec_ref {
		void operator()(fs_fcb_t *ptr) noexcept {
			fs_dec_fcb_ref(ptr);
		}
	};

	using fcb_ptr_t = kfxx::custom_rc_ptr_t<fs_fcb_t, fcb_inc_ref, fcb_dec_ref>;
}

#endif

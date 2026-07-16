#ifndef _PBOS_KI_KM_STRING_HH_
#define _PBOS_KI_KM_STRING_HH_

#include <pbos/km/string.h>
#include <pbos/kfxx/string_view.hh>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/ps/mutex.hh>

struct ki_shared_string_t final : public kfxx::rbtree_t<kfxx::string_view>::node_t {
	size_t len = 0;
	size_t ref_count = 0;
};

extern kfxx::rbtree_t<kfxx::string_view> ki_registered_shared_strings;
extern ps::mutex_t ki_shared_string_mutex;

#endif

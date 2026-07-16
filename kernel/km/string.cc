#include <pbos/kf/atomic.h>
#include <pbos/mm/mm.h>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/string.hh>

kfxx::rbtree_t<kfxx::string_view> ki_registered_shared_strings;
ps::mutex_t ki_shared_string_mutex;

PBOS_API km_result_t km_register_shared_string(const char *str, size_t len, const char **str_out, km_shared_string_handle_t *handle_out) {
	ps::mutex_guard mg(ki_shared_string_mutex);
	if (auto it = ki_registered_shared_strings.find(kfxx::string_view(str, len)); it) {
		kf_atomic_inc_size(&static_cast<ki_shared_string_t *>(it)->ref_count);
		*str_out = it->rb_value.data();
		return KM_RESULT_OK;
	}

	char *data = static_cast<char *>(kfxx::kernel_allocator()->alloc(len, alignof(char)));
	if (!data)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard g([data, len]() noexcept {
		kfxx::kernel_allocator()->release(data, len, alignof(char));
	});

	ki_shared_string_t *s = kfxx::alloc_and_construct<ki_shared_string_t>(kfxx::kernel_allocator());

	if (!s)
		return KM_RESULT_NO_MEM;

	s->rb_value = kfxx::string_view(data, len);

	ki_registered_shared_strings.insert_unwrap(s);

	g.release();

	kf_atomic_inc_size(&s->ref_count);

	if (str_out)
		*str_out = data;

	*handle_out = static_cast<km_shared_string_handle_t>(s);

	return KM_RESULT_OK;
}

PBOS_API const char *km_lookup_shared_string(km_shared_string_handle_t handle, size_t *length_out) {
	auto reg = static_cast<ki_shared_string_t *>(handle);
	kf_atomic_inc_size(&reg->ref_count);
	*length_out = reg->len;
	return reg->rb_value.data();
}

PBOS_API void km_ref_shared_string(km_shared_string_handle_t handle) {
	kf_atomic_inc_size(&static_cast<ki_shared_string_t *>(handle)->ref_count);
}

PBOS_API void km_unref_shared_string(km_shared_string_handle_t handle) {
	ps::mutex_guard mg(ki_shared_string_mutex);

	ki_shared_string_t *s = static_cast<ki_shared_string_t *>(handle);
	if (!kf_atomic_dec_size(&s->ref_count)) {
		ki_registered_shared_strings.remove(s);
		kfxx::destroy_and_release<ki_shared_string_t>(kfxx::kernel_allocator(), s);
	}
}

#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/hashmap.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kfxx/unique_ptr.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/ps/exec.hh>
#include <pbos/ki/ps/kmod.hh>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

_ps_kmod_t::_ps_kmod_t(kfxx::allocator_t *allocator) : registered_symbols(allocator) {
}

_ps_kmod_t::~_ps_kmod_t() {
}

ps_kmod_t *ki_ps_kmod_list = nullptr;
ps::mutex_t ki_ps_kmod_list_mutex;

PBOS_API km_result_t ps_load_kmod(fs_fcb_t *file_fp, ps_kmod_t **kmod_out) {
	km_result_t result;

	for (auto it = ki_registered_binldrs.begin(); it != ki_registered_binldrs.end(); ++it) {
		ps_kmod_t *kmod;
		KM_FAILED(ps_create_kmod(&kmod));
		kfxx::scope_guard free_kmod_guard([kmod]() noexcept {
			ps_destroy_kmod(kmod);
		});

		if (KM_SUCCESS(result = static_cast<ki_binldr_registry_t *>(it.node)->ops.load_kmod(kmod, file_fp))) {
			if (!kmod->init_fn)
				continue;
			if (!kmod->deinit_fn)
				continue;
			KM_RETURN_IF_FAILED(kmod->init_fn());
			free_kmod_guard.release();
			return result;
		}

		if (result != KM_RESULT_UNSUPPORTED_EXECFMT)
			return result;
	}

	return KM_RESULT_UNSUPPORTED_EXECFMT;
}

PBOS_API void ps_destroy_kmod(ps_kmod_t *kmod) {
	{
		ps::mutex_guard g(ki_ps_kmod_list_mutex.c_mutex());

		if (ki_ps_kmod_list == kmod)
			ki_ps_kmod_list = kmod->next;

		if (kmod->prev)
			kmod->prev->next = kmod->next;
		if (kmod->next)
			kmod->next->prev = kmod->prev;
	}

	if (kmod->is_inited) {
		if (kmod->deinit_fn)
			kmod->deinit_fn();
	}

	while (kmod->registered_symbols.size()) {
		auto i = kmod->registered_symbols.begin();
		ps_unregister_kernel_symbol(i->data(), i->size());
		kmod->registered_symbols.remove(i);
	}

	while (kmod->registered_sections.size())
		ps_remove_section_from_kmod(kmod, static_cast<ps_kmod_section_t *>(kmod->registered_sections.begin().node));

	kfxx::destroy_and_release<ps_kmod_t>(kfxx::kernel_allocator(), kmod);
}

PBOS_API km_result_t ps_create_kmod(ps_kmod_t **kmod_out) {
	ps_kmod_t *kmod = kfxx::alloc_and_construct<ps_kmod_t>(kfxx::kernel_allocator(), kfxx::kernel_allocator());

	if (!kmod)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard destroy_kmod_guard([kmod]() noexcept {
		ps_destroy_kmod(kmod);
	});

	ps::mutex_guard g(ki_ps_kmod_list_mutex.c_mutex());

	if (ki_ps_kmod_list) {
		ki_ps_kmod_list->prev = kmod;
	}
	kmod->next = ki_ps_kmod_list;
	ki_ps_kmod_list = kmod;

	destroy_kmod_guard.release();

	*kmod_out = kmod;

	return KM_RESULT_OK;
}

PBOS_API km_result_t ps_add_section_to_kmod(ps_kmod_t *kmod, void *vaddr, size_t size, ps_kmod_section_t **section_out) {
	KM_RETURN_IF_FAILED(ki_ps_alloc_kmod_section(vaddr, size, kmod, section_out));
	return KM_RESULT_OK;
}

PBOS_API void ps_remove_section_from_kmod(ps_kmod_t *kmod, ps_kmod_section_t *section) {
	kd_dbgcheck(kmod, "The kmod for %s must not be empty", __func__);
	kd_dbgcheck(section, "The section for %s must not be empty", __func__);
	if (section->parent_mod != kmod)
		km_panic("Removing section %p with parent %p which is mismatched", section->rb_value, kmod);
	ki_ps_destroy_kmod_section(section);
}

km_result_t ki_ps_alloc_kmod_section(void *vaddr, size_t size, ps_kmod_t *kmod, ps_kmod_section_t **section_out) {
	ps_kmod_section_t *section = kfxx::alloc_and_construct<ps_kmod_section_t>(kfxx::kernel_allocator());

	if (!section)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard destroy_section_guard([section]() noexcept {
		ki_ps_destroy_kmod_section(section);
	});

	section->rb_value = vaddr;
	section->size = size;

	if (!kmod->registered_sections.insert(section))
		return KM_RESULT_EXISTED;

	section->parent_mod = kmod;

	destroy_section_guard.release();

	if (section_out)
		*section_out = section;

	return KM_RESULT_OK;
}

void ki_ps_destroy_kmod_section(ps_kmod_section_t *section) {
	if (section->parent_mod)
		section->parent_mod->registered_sections.remove(section);

	kfxx::destroy_and_release<ps_kmod_section_t>(kfxx::kernel_allocator(), section);
}

kfxx::hashmap_t<kfxx::string_view, ki_kernel_symbol_t *> ki_registered_kernel_symbols(kfxx::kernel_allocator());
kfxx::rbtree_t<void *> ki_registered_kernel_symbol_query_tree;

void ki_destroy_kernel_symbol(ki_kernel_symbol_t *sym) {
	if (sym->name)
		kfxx::kernel_allocator()->release(sym->name, sym->name_len, alignof(char));
	kfxx::destroy_and_release<ki_kernel_symbol_t>(kfxx::kernel_allocator(), sym);
}

PBOS_API void ps_set_kmod_init_fn(ps_kmod_t *kmod, ps_kmod_init_fn_t init_fn) {
	kmod->init_fn = init_fn;
}

PBOS_API void ps_set_kmod_deinit_fn(ps_kmod_t *kmod, ps_kmod_deinit_fn_t deinit_fn) {
	kmod->deinit_fn = deinit_fn;
}

PBOS_API km_result_t ps_register_kernel_symbol(ps_kmod_t *kmod, const char *name, size_t name_len, void *addr, size_t len) {
	ki_kernel_symbol_t *sym;
	KM_RETURN_IF_FAILED(ki_do_register_kernel_symbol(name, name_len, addr, len, &sym));

	kfxx::scope_guard unregister_symbol_guard([name, name_len]() noexcept {
		ps_unregister_kernel_symbol(name, name_len);
	});

	if (!kmod->registered_symbols.insert(kfxx::string_view(sym->name, sym->name_len)))
		return KM_RESULT_NO_MEM;

	unregister_symbol_guard.release();
	return KM_RESULT_OK;
}

km_result_t ki_do_register_kernel_symbol(const char *name, size_t name_len, void *addr, size_t len, ki_kernel_symbol_t **symbol_out) {
	if (!ki_registered_kernel_symbols.shrink_buckets())
		return KM_RESULT_NO_MEM;

	if (auto node = ki_registered_kernel_symbol_query_tree.find_max_lteq(addr); node) {
		if (((char *)node->rb_value) + static_cast<ki_kernel_symbol_t *>(node)->len >= addr)
			return KM_RESULT_EXISTED;
	}

	ki_kernel_symbol_t *sym = kfxx::alloc_and_construct<ki_kernel_symbol_t>(kfxx::kernel_allocator());
	kfxx::scope_guard release_sym_guard([sym]() noexcept {
		ki_destroy_kernel_symbol(sym);
	});

	if (!(sym->name = (char *)kfxx::kernel_allocator()->alloc(name_len, alignof(char))))
		return KM_RESULT_NO_MEM;
	sym->name_len = name_len;

	memcpy(sym->name, name, name_len);

	sym->rb_value = addr;
	sym->len = len;

	if (symbol_out)
		*symbol_out = sym;

	if (!ki_registered_kernel_symbols.insert(kfxx::string_view(sym->name, sym->name_len), +sym))
		return KM_RESULT_NO_MEM;

	release_sym_guard.release();

	return KM_RESULT_OK;
}

PBOS_API km_result_t ps_unregister_kernel_symbol(const char *name, size_t name_len) {
	kfxx::string_view name_view = kfxx::string_view(name, name_len);
	if (auto it = ki_registered_kernel_symbols.find(name_view); it != ki_registered_kernel_symbols.end()) {
		ki_destroy_kernel_symbol(it.value());
		ki_registered_kernel_symbols.remove(name_view);
		return KM_RESULT_OK;
	}
	return KM_RESULT_NOT_FOUND;
}

PBOS_API void *ps_get_kernel_symbol(const char *name, size_t name_len) {
	kfxx::string_view name_view = kfxx::string_view(name, name_len);
	if (auto it = ki_registered_kernel_symbols.find(name_view); it != ki_registered_kernel_symbols.end()) {
		return it.value()->rb_value;
	}
	return nullptr;
}

PBOS_EXTERN_C_END

#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/ps/exec.hh>
#include <pbos/ki/ps/kmod.hh>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

ps_kmod_t *ki_ps_kmod_list = nullptr;
ps::mutex_t ki_ps_kmod_list_mutex;

PBOS_API km_result_t ps_load_kmod(fs_fcb_t *file_fp, ps_kmod_t **kmod_out) {
	km_result_t result;

	for (auto it = ki_registered_binldrs.begin(); it != ki_registered_binldrs.end(); ++it) {
		if (KM_SUCCESS(result = static_cast<ki_binldr_registry_t *>(it.node)->ops.load_kmod(file_fp))) {
			return result;
		}

		if (result != KM_RESULT_UNSUPPORTED_EXECFMT)
			return result;
	}

	return KM_RESULT_UNSUPPORTED_EXECFMT;
}

PBOS_API void ps_remove_kmod(ps_kmod_t *kmod) {
	ps::mutex_guard g(ki_ps_kmod_list_mutex.c_mutex());

	if (ki_ps_kmod_list == kmod)
		ki_ps_kmod_list = kmod->next;

	if (kmod->prev)
		kmod->prev->next = kmod->next;
	if (kmod->next)
		kmod->next->prev = kmod->prev;

	while (kmod->registered_sections.size())
		ps_remove_section_from_kmod(kmod, static_cast<ps_kmod_section_t *>(kmod->registered_sections.begin().node));

	kfxx::destroy_and_release<ps_kmod_t>(kfxx::kernel_allocator(), kmod);
}

PBOS_API km_result_t ps_create_kmod(ps_kmod_t **kmod_out) {
	ps_kmod_t *kmod = kfxx::alloc_and_construct<ps_kmod_t>(kfxx::kernel_allocator());

	if (!kmod)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard destroy_kmod_guard([kmod]() noexcept {
		ps_remove_kmod(kmod);
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

	return KM_RESULT_OK;
}

void ki_ps_destroy_kmod_section(ps_kmod_section_t *section) {
	if (section->parent_mod)
		section->parent_mod->registered_sections.remove(section);

	kfxx::destroy_and_release<ps_kmod_section_t>(kfxx::kernel_allocator(), section);
}

PBOS_EXTERN_C_END

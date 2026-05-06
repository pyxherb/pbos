#include <hal/x86_64/proc.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kn/km/exec.hh>
#include <pbos/kfxx/uuid.hh>
#include <string.h>

PBOS_EXTERN_C_BEGIN

kfxx::rbtree_t<kf_uuid_t> ki_registered_binldrs;
kfxx::rbtree_t<fs_fcb_t *> ki_registered_binprotos;

static ps_proc_id_t _last_proc_id = 0;

ps_proc_id_t ki_alloc_proc_id() {
	return _last_proc_id++;
}

km_result_t km_register_binldr(kf_uuid_t *uuid, km_binldr_t *binldr) {
	ki_binldr_registry_t *reg = (ki_binldr_registry_t *)mm_kalloc(sizeof(ki_binldr_registry_t), alignof(ki_binldr_registry_t));
	if (!reg)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);

	// Initialize the registry
	kfxx::construct_at<ki_binldr_registry_t>(reg);

	memcpy(&reg->rb_value, &uuid, sizeof(*uuid));
	memcpy(&reg->binldr, binldr, sizeof(km_binldr_t));

	ki_registered_binldrs.insert(reg);

	return KM_RESULT_OK;
}

km_result_t km_exec(
	ps_proc_id_t parent,
	se_uid_t uid,
	fs_fcb_t *file_fp,
	ps_proc_id_t *pid_out) {
	km_result_t result;

	ps_proc_id_t new_proc_id = ki_alloc_proc_id();
	if (new_proc_id < 0)
		return KM_MAKEERROR(KM_RESULT_NO_SLOT);

	ps_pcb_t *pcb = ps_alloc_pcb();
	if (!pcb) {
		result = KM_MAKEERROR(KM_RESULT_NO_MEM);
		goto failed;
	}

	for (auto it = ki_registered_binldrs.begin(); it != ki_registered_binldrs.end(); ++it) {
		if (KM_SUCCEEDED(result = static_cast<ki_binldr_registry_t *>(it.node)->binldr.load_exec(pcb, file_fp))) {
			io::irq_disable_lock irq_disable_lock;
			pcb->rb_value = ki_alloc_proc_id();
			ps_create_proc(pcb, parent);
			return KM_RESULT_OK;
		}

		if (result != KM_RESULT_UNSUPPORTED_EXECFMT) {
			goto failed;
		}
	}

	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_EXECFMT);
failed:
	if (pcb) {
		// TODO: Release PCB.
	}

	return result;
}

km_result_t km_register_binproto(fs_fcb_t *fcb, km_binproto_t **proto_out) {
	io::irq_disable_lock irq_disable_lock;

	if (ki_registered_binprotos.find(fcb))
		return KM_MAKEERROR(KM_RESULT_EXISTED);

	km_binproto_t *proto = (km_binproto_t *)mm_kalloc(sizeof(km_binproto_t), alignof(km_binproto_t));

	kfxx::construct_at<km_binproto_t>(proto);

	proto->rb_value = fcb;

	ki_registered_binprotos.insert(proto);

	*proto_out = proto;

	return KM_RESULT_OK;
}

km_binproto_t *km_find_binproto(fs_fcb_t *fcb) {
	io::irq_disable_lock irq_disable_lock;

	return static_cast<km_binproto_t *>(ki_registered_binprotos.find(fcb));
}

void km_unregister_binproto(km_binproto_t *proto) {
	io::irq_disable_lock irq_disable_lock;

	// TODO: Check if the prototype is registered.
	ki_registered_binprotos.remove(proto);
}

km_result_t km_add_segment_to_binproto(km_binproto_t *proto, void *vaddr_base, size_t size, mm_pgaccess_t pgaccess, km_binseg_t **seg_out) {
	if (vaddr_base != (void *)PGFLOOR(vaddr_base))
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	km_binseg_t *seg = (km_binseg_t *)mm_kalloc(sizeof(km_binseg_t), alignof(km_binseg_t));
	kfxx::scope_guard seg_release_guard([seg]() noexcept {
		mm_kfree(seg);
	});

	kfxx::construct_at<km_binseg_t>(seg);

	seg->vaddr_base = vaddr_base;
	seg->size = size;
	seg->access = pgaccess;
	seg->cur_offset = 0;

	kfxx::scope_guard release_page_pools_guard([seg]() noexcept {
		for (km_binseg_page_pool_t *p = seg->pages, *next; p;) {
			next = p->next;
			for (size_t i = 0; i < p->used_num; ++i) {
				mm_pgfree(p->descs[i].rb_value);
				seg->page_descs_query_tree.remove(&p->descs[i]);
			}
			mm_kfree(p);
		}
	});
	while (seg->cur_offset < size) {
		km_binseg_page_pool_t *page_pool = (km_binseg_page_pool_t *)mm_kalloc(sizeof(km_binseg_page_pool_t), PAGESIZE);
		kfxx::scope_guard release_cur_page_pool_guard([seg, page_pool]() noexcept {
			for (size_t i = 0; i < page_pool->used_num; ++i) {
				mm_pgfree(page_pool->descs[i].rb_value);
				seg->page_descs_query_tree.remove(&page_pool->descs[i]);
			}
			mm_kfree(page_pool);
		});

		kfxx::construct_at<km_binseg_page_pool_t>(page_pool);

		for (size_t i = 0; i < PBOS_ARRAYSIZE(page_pool->descs); ++i) {
			kfxx::construct_at<km_binseg_page_desc_t>(&page_pool->descs[i]);

			void *paddr = mm_pgalloc(MM_PHYSICAL_MEMORY_TYPE_AVAILABLE);

			if (!paddr)
				return KM_MAKEERROR(KM_RESULT_NO_MEM);

			page_pool->descs[i].rb_value = paddr;
			seg->page_descs_query_tree.insert(&page_pool->descs[i]);

			++page_pool->used_num;
			if ((seg->cur_offset += PAGESIZE) >= size)
				break;
		}
		release_cur_page_pool_guard.release();

		if (!seg->cur_page_pool) {
			seg->pages = page_pool;
			seg->cur_page_pool = page_pool;
		} else {
			seg->cur_page_pool->next = page_pool;
			page_pool->prev = seg->cur_page_pool;

			seg->cur_page_pool = page_pool;
		}
	}

	release_page_pools_guard.release();
	seg_release_guard.release();

	*seg_out = seg;

	return KM_RESULT_OK;
}

void *km_get_paddr_in_binseg(km_binseg_t *seg, void *vaddr) {
	return seg->page_descs_query_tree.find(vaddr);
}

PBOS_EXTERN_C_END

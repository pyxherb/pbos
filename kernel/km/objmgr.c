#include <pbos/kf/string.h>
#include <pbos/km/logger.h>
#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kn/km/objmgr.h>

om_class_t *kn_class_list = NULL;
kf_rbtree_t kn_global_handles;
om_handle_t kn_last_handle = 0;

static kf_rbtree_t kn_unused_objects;

static bool _kn_object_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	return x < y;
}

static void _kn_unused_object_nodefree(kf_rbtree_node_t *p) {
	om_object_t *_p = PB_CONTAINER_OF(om_object_t, tree_header, p);
	_p->p_class->destructor(_p);
}

om_class_t *om_register_class(uuid_t *uuid, om_destructor_t destructor) {
	om_class_t *cls = mm_kmalloc(sizeof(om_class_t));
	if (!cls)
		return NULL;

	memcpy(&cls->uuid, uuid, sizeof(uuid_t));
	cls->destructor = destructor;
	cls->obj_num = 0;

	// Prepend to the list.
	if (kn_class_list)
		kn_class_list->last = cls;
	cls->next = kn_class_list;
	kn_class_list = cls;

	return cls;
}

void om_unregister_class(om_class_t *cls) {
	if (!cls)
		km_panic("Unregistering a kernel class with NULL");
	if (cls->obj_num)
		km_panic("Unregistering kernel class %p during the objects are not all released");

	if (kn_class_list == cls)
		kn_class_list = cls->next;

	if (cls->last)
		cls->last->next = cls->next;
	if (cls->next)
		cls->next->last = cls->last;

	mm_kfree(cls);
}

bool om_is_class_registered(om_class_t *cls) {
	for (om_class_t *i = kn_class_list; i; i = i->next)
		if (i == cls)
			return true;
	return false;
}

om_class_t *om_lookup_class(uuid_t *uuid) {
	for (om_class_t *i = kn_class_list; i; i = i->next)
		if (uuid_eq(&i->uuid, uuid))
			return i;
	return NULL;
}

void om_init_object(om_object_t *obj, om_class_t *cls) {
	obj->ref_num = 0;
	obj->handle_num = 0;
	obj->p_class = cls;
	cls->obj_num++;
}

void om_incref(om_object_t *obj) {
	if ((!obj->ref_num++) && (!obj->handle_num))
		kf_rbtree_remove(&kn_unused_objects, &obj->tree_header);
}

void om_decref(om_object_t *obj) {
	if (--obj->ref_num)
		obj->p_class->destructor(obj);
}

static bool _kn_handle_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const kn_handle_registry_t *_x = PB_CONTAINER_OF(kn_handle_registry_t, tree_header, x),
							   *_y = PB_CONTAINER_OF(kn_handle_registry_t, tree_header, y);
	return _x->handle < _y->handle;
}

static void _kn_handle_nodefree(kf_rbtree_node_t *p) {
	kn_handle_registry_t *_p = PB_CONTAINER_OF(kn_handle_registry_t, tree_header, p);
	--_p->object->ref_num;
	mm_kfree(_p);
}

kn_handle_registry_t *kn_lookup_handle_registry(om_handle_t handle) {
	kn_handle_registry_t find_node = {
		.handle = handle
	};
	kf_rbtree_node_t *node = kf_rbtree_find(&kn_global_handles, &(find_node.tree_header));
	if (node)
		return PB_CONTAINER_OF(kn_handle_registry_t, tree_header, node);
	return NULL;
}

km_result_t om_create_handle(om_object_t *obj, om_handle_t *handle_out) {
	// Find a free handle.
	om_handle_t initial_handle = kn_last_handle;
	while (kn_lookup_handle_registry(++kn_last_handle)) {
		if (kn_last_handle == initial_handle)
			return KM_MAKEERROR(KM_RESULT_NO_SLOT);
	}

	kn_handle_registry_t *registry = mm_kmalloc(sizeof(kn_handle_registry_t));

	memset(registry, 0, sizeof(kn_handle_registry_t));

	registry->handle = kn_last_handle;
	registry->object = obj;

	km_result_t result;

	if (KM_FAILED(result = kf_rbtree_insert(&kn_global_handles, &registry->tree_header))) {
		mm_kfree(registry);
		return result;
	}

	++obj->handle_num;
	if (!obj->ref_num && (!obj->handle_num))
		kf_rbtree_remove(&kn_unused_objects, &obj->tree_header);

	*handle_out = kn_last_handle;

	if (kn_last_handle == OM_HANDLE_MAX)
		++kn_last_handle;

	return KM_RESULT_OK;
}

km_result_t om_close_handle(om_handle_t handle) {
	kn_handle_registry_t *registry = kn_lookup_handle_registry(handle);
	if (!registry)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);
	kf_rbtree_remove(&kn_global_handles, &registry->tree_header);
	return KM_RESULT_OK;
}

km_result_t om_duplicate_handle(om_handle_t handle, om_handle_t *handle_out) {
	kn_handle_registry_t *registry = kn_lookup_handle_registry(handle);
	if (!registry)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);
	return om_create_handle(registry->object, handle_out);
}

void om_gc() {
	kf_rbtree_clear(&kn_unused_objects);
}

km_result_t om_deref_handle(om_handle_t handle, om_object_t **obj_out) {
	kn_handle_registry_t *registry = kn_lookup_handle_registry(handle);
	if (!registry) {
		kdprintf("Dereferencing an invalid handle: %u\n", handle);
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);
	}
	*obj_out = registry->object;
	return KM_RESULT_OK;
}

void om_init() {
	kf_rbtree_init(
		&kn_global_handles,
		_kn_handle_nodecmp,
		_kn_handle_nodefree);
	kf_rbtree_init(
		&kn_unused_objects,
		_kn_object_nodecmp,
		_kn_unused_object_nodefree);
}

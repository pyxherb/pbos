#include <pbos/kf/string.h>
#include <pbos/km/logger.h>
#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kn/km/objmgr.h>

om_class_t *kn_class_list = NULL;

static kf_rbtree_t kn_unused_objects;

static bool _kn_object_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	return x < y;
}

static void _kn_unused_object_nodefree(kf_rbtree_node_t *p) {
	om_object_t *_p = PBOS_CONTAINER_OF(om_object_t, tree_header, p);
	_p->prop.p_class->destructor(_p);
}

om_class_t *om_register_class(kf_uuid_t *uuid, om_destructor_t destructor) {
	om_class_t *cls = mm_kmalloc(sizeof(om_class_t), alignof(om_class_t));
	if (!cls)
		return NULL;

	memcpy(&cls->uuid, uuid, sizeof(kf_uuid_t));
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

om_class_t *om_lookup_class(kf_uuid_t *uuid) {
	for (om_class_t *i = kn_class_list; i; i = i->next)
		if (uuid_eq(&i->uuid, uuid))
			return i;
	return NULL;
}

void om_init_object(om_object_t *obj, om_class_t *cls, om_object_flags_t flags) {
	obj->ref_num = 0;
	obj->prop.p_class = cls;
	obj->prop.flags = OM_OBJECT_KERNEL | flags;
	cls->obj_num++;
}

void om_incref(om_object_t *obj) {
	++obj->ref_num;
}

void om_decref(om_object_t *obj) {
	if (!(--obj->ref_num)) {
		--obj->prop.p_class->obj_num;
		obj->prop.p_class->destructor(obj);
	}
}

void om_deinit_object(om_object_t *obj) {
	--obj->prop.p_class->obj_num;
	obj->ref_num = 0;
}

void om_gc() {
	kf_rbtree_clear(&kn_unused_objects);
}

void om_init() {
	kf_rbtree_init(
		&kn_unused_objects,
		_kn_object_nodecmp,
		_kn_unused_object_nodefree);
}

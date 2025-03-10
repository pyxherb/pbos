#ifndef _PBOS_KM_OBJMGR_H_
#define _PBOS_KM_OBJMGR_H_

#include <pbos/attribs.h>
#include <pbos/common.h>
#include <pbos/kf/rbtree.h>
#include <pbos/kf/uuid.h>
#include "result.h"

#define OM_HANDLE_MIN 1
#define OM_HANDLE_MAX UINT32_MAX

typedef struct _om_object_t om_object_t;

/// @brief Type for destructor functions.
/// @note The destructor should release the object.
typedef void (*om_destructor_t)(om_object_t *obj);

typedef uint32_t om_class_flags_t;

/// @brief Class registry.
typedef struct _om_class_t {
	uuid_t uuid;
	om_destructor_t destructor;
	size_t obj_num;
	struct _om_class_t *next, *last;
} om_class_t;

#define OM_OBJECT_KERNEL 0x00000001

typedef uint32_t om_object_flags_t;

typedef struct _om_object_prop_t {
	om_class_t *p_class;
	om_object_flags_t flags;
} om_object_prop_t;

/// @brief Object header.
typedef struct _om_object_t {
	kf_rbtree_node_t tree_header;
	size_t ref_num;
	om_object_prop_t prop;
} om_object_t;

///
/// @brief Register a kernel class.
///
/// @param uuid UUID of the kernel class to be registered.
/// @param destructor Destructor of the kernel class.
/// @return Registered kernel class registry, NULL if failed.
///
om_class_t *om_register_class(uuid_t *uuid, om_destructor_t destructor);

///
/// @brief Unregister a kernel class.
///
/// @param cls Pointer to the class registry.
///
void om_unregister_class(om_class_t *cls);

///
/// @brief Check if a kernel class was registered.
///
/// @param cls Class registry to be checked.
/// @return true The class was registered.
/// @return false The class was not registered.
///
bool om_is_class_registered(om_class_t *cls);
om_class_t *om_lookup_class(uuid_t *uuid);

///
/// @brief Create a new kernel object with specified kernel class.
///
/// @param cls Class of new object.
/// @return Pointer to the new object.
///
void om_init_object(om_object_t *obj, om_class_t *cls, om_object_flags_t flags);

///
/// @brief Increase reference count of a kernel object.
///
/// @param obj Target object.
///
void om_incref(om_object_t *obj);

///
/// @brief Decrease reference count of a kernel object.
///
/// @param obj Target object.
///
void om_decref(om_object_t *obj);

void om_deinit_object(om_object_t *obj);

#define om_refcountof(o) (const size_t)((o)->ref_num)
#define om_classof(o) ((o)->p_class)

#define om_is_cachable(obj) ((obj)->ref_num == 0)

void om_gc();

#endif

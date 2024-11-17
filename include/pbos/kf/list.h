#ifndef _PBOS_KM_LIST_H_
#define _PBOS_KM_LIST_H_

#include <pbos/attribs.h>
#include <pbos/common.h>

typedef struct _kf_list_node_t {
	struct _kf_list_node_t *last, *next;
} kf_list_node_t;

#define kf_list_init(d, p, n) (d)->last = (p), (d)->next = (n)
#define kf_list_prepend(dest, node)                  \
	if ((dest)->last) (dest)->last->next = (node); \
	(node)->last = (dest)->last;                   \
	(node)->next = (dest);                         \
	(dest)->last = (node)
#define kf_list_append(dest, node)                    \
	if ((dest)->next) (dest)->next->last = (node); \
	(node)->next = (dest)->next;                   \
	(node)->last = (dest);                         \
	(dest)->next = (node)
#define kf_list_remove(dest)                          \
	if ((dest)->last) (dest)->last->next = (dest)->next; \
	if ((dest)->next) (dest)->next->last = (dest)->last
#define kf_list_last(node) ((node)->last)
#define kf_list_next(node) ((node)->next)

#define kf_list_foreach(i, node) for (kf_list_node_t *i = (node); i; i = i->next)

PB_FORCEINLINE static kf_list_node_t *kf_list_head(kf_list_node_t *i) {
	while (i->last)
		i = i->last;
	return i;
}

PB_FORCEINLINE static kf_list_node_t *kf_list_end(kf_list_node_t *i) {
	while (i->next)
		i = i->next;
	return i;
}

#endif

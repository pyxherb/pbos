#ifndef HAL_I386_MM_SKID_VENTRY_H_
#define HAL_I386_MM_SKID_VENTRY_H_

#include <hal/i386/mm.h>

/// @brief Each instance represents a single fragment in physical or virtual memory space.
typedef struct _skid_frag_t {
	void *vaddr, *paddr;
	size_t size;
} skid_frag_t;

typedef struct _skid_fragpg_t {
	struct _skid_fragpg_t *last, *next;
	uint16_t inuse_num;
	skid_frag_t entries[(PAGESIZE - sizeof(void *) * 2 - sizeof(uint16_t)) / sizeof(skid_frag_t)];
} skid_fragpg_t;

skid_fragpg_t *skid_fragpg_new();
void skid_fragpg_free(skid_fragpg_t *pg);
skid_frag_t *skid_frag_slotalloc();
skid_frag_t *skid_frag_alloc(const void *vaddr, const void *paddr, size_t size);
skid_frag_t *skid_frag_get_exact(const void *ptr);
skid_frag_t *skid_frag_get(const void *ptr, size_t size);
skid_frag_t *skid_frag_get_by_paddr(const void *ptr, size_t size);
void skid_frag_free(skid_frag_t* frag);
#define skid_fragpg_foreach(i) for (skid_fragpg_t *i = skid_frag_list; i; i = i->next)
#define skid_frag_foreach(pg, i) for (uint16_t i = 0; i < ARRAYLEN((pg)->entries); ++i)

#define skid_fragpg_isfree(pg) (!(pg)->inuse_num)
#define skid_fragpg_isfull(pg) ((pg)->inuse_num >= ARRAYLEN(pg->entries))

#define skid_frag_isvalid(chunk) ((chunk)->vaddr)

#define skid_fragpg_of(frag) ((skid_fragpg_t *)(((size_t)frag) & ~PGOFF_MAX))

// Frag list
extern skid_fragpg_t *skid_frag_list;

#endif

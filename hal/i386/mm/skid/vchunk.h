#ifndef HAL_I386_MM_SKID_VCHUNK_H_
#define HAL_I386_MM_SKID_VCHUNK_H_

#include <hal/i386/mm.h>

/// @brief Each instance represents an allocated paged virtual memory area.
typedef struct _skid_vchunk_t {
	void *ptr;
	pgsize_t pg_num;
	uint32_t ref_num;
} skid_vchunk_t;

typedef struct _skid_vchunkpg_t {
	struct _skid_vchunkpg_t *last, *next;
	uint16_t inuse_num;
	skid_vchunk_t chunks[(PAGESIZE - sizeof(void *) * 2 - sizeof(uint16_t)) / sizeof(skid_vchunk_t)];
} skid_vchunkpg_t;

skid_vchunkpg_t *skid_vchunkpg_new();
void skid_vchunkpg_free(skid_vchunkpg_t *pg);
skid_vchunk_t *skid_vchunk_slotalloc();
skid_vchunk_t *skid_vchunk_alloc(size_t size);
skid_vchunk_t *skid_vchunk_get(const void *ptr, size_t size);

void skid_vchunk_incref(skid_vchunk_t *chunk);
void skid_vchunk_decref(skid_vchunk_t *chunk);

#define skid_vchunkpg_foreach(i) for (skid_vchunkpg_t *i = skid_vchunk_list; i; i = i->next)
#define skid_vchunk_foreach(pg, i) for (uint16_t i = 0; i < ARRAYLEN((pg)->chunks); ++i)

#define skid_vchunkpg_isfree(pg) (!(pg)->inuse_num)
#define skid_vchunkpg_isfull(pg) ((pg)->inuse_num >= ARRAYLEN(pg->chunks))

#define skid_vchunk_isvalid(chunk) ((chunk)->ptr)

#define skid_vchunkpg_of(vchunk) ((skid_vchunkpg_t *)(((size_t)vchunk) & ~PGOFF_MAX))

void skid_vchunk_scan_and_merge();

// Virtual chunk list
extern skid_vchunkpg_t *skid_vchunk_list;

#endif

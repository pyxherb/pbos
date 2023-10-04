#ifndef HAL_I386_MM_SKID_PCHUNK_H_
#define HAL_I386_MM_SKID_PCHUNK_H_

#include <hal/i386/mm.h>

typedef struct _skid_pchunk_t {
	void *ptr;
	uint8_t order;
	uint32_t ref_num;	// Number of virtual memory chunks that referenced it.
} skid_pchunk_t;

typedef struct _skid_pchunkpg_t {
	struct _skid_pchunkpg_t *last, *next;
	uint16_t inuse_num;
	uint8_t flags;
	skid_pchunk_t chunks[(PAGESIZE - sizeof(void *) * 2 - sizeof(uint16_t) - sizeof(uint8_t)) / sizeof(skid_pchunk_t)];
} skid_pchunkpg_t;

skid_pchunkpg_t *skid_pchunkpg_new();
void skid_pchunkpg_free(skid_pchunkpg_t *pg);
skid_pchunk_t *skid_pchunk_slotalloc();
skid_pchunk_t *skid_pchunk_alloc(uint8_t order);
skid_pchunk_t *skid_pchunk_get(const void *ptr, size_t size);

void skid_pchunk_incref(skid_pchunk_t *chunk);

void skid_pchunk_decref(skid_pchunk_t *chunk);

#define skid_pchunkpg_foreach(i) for (skid_pchunkpg_t *i = skid_pchunk_list; i; i = i->next)
#define skid_pchunk_foreach(pg, i) for (uint16_t i = 0; i < ARRAYLEN((pg)->chunks); ++i)

#define skid_pchunkpg_isfree(pg) (!(pg)->inuse_num)
#define skid_pchunkpg_isfull(pg) ((pg)->inuse_num >= ARRAYLEN((pg)->chunks))

#define skid_pchunk_isvalid(chunk) ((chunk)->ptr)

#define skid_pchunkpg_of(pchunk) ((skid_pchunkpg_t *)(((size_t)pchunk) & ~PGOFF_MAX))

// Physical chunk list
extern skid_pchunkpg_t *skid_pchunk_list;

#endif

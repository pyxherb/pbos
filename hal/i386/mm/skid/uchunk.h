#ifndef HAL_I386_MM_SKID_UCHUNK_H_
#define HAL_I386_MM_SKID_UCHUNK_H_

#include <hal/i386/mm.h>

typedef struct _skid_uchunk_t {
	void* ptr;
	size_t size;
} skid_uchunk_t;

typedef struct _skid_uchunkpg_t {
	struct _skid_uchunkpg_t *last, *next;
	uint16_t inuse_num;
	skid_uchunk_t chunks[(PAGESIZE - sizeof(void *) * 2 - sizeof(uint16_t)) / sizeof(skid_uchunk_t)];
} skid_uchunkpg_t;

skid_uchunkpg_t *skid_uchunkpg_new();
void skid_uchunkpg_free(skid_uchunkpg_t *pg);
skid_uchunk_t *skid_uchunk_slotalloc();
skid_uchunk_t *skid_uchunk_alloc(void *ptr, size_t size);
skid_uchunk_t *skid_uchunk_get(const void *ptr, size_t size);
skid_uchunk_t *skid_uchunk_get_exact(const void *ptr);
void skid_uchunk_free(skid_uchunk_t* uchunk);

#define skid_uchunkpg_foreach(i) for (skid_uchunkpg_t *i = skid_uchunk_list; i; i = i->next)
#define skid_uchunk_foreach(pg, i) for (uint16_t i = 0; i < ARRAYLEN((pg)->chunks); ++i)

#define skid_uchunkpg_isfree(pg) (!(pg)->inuse_num)
#define skid_uchunkpg_isfull(pg) ((pg)->inuse_num >= ARRAYLEN((pg)->chunks))

#define skid_uchunk_isvalid(chunk) ((chunk)->ptr)

#define skid_uchunkpg_of(uchunk) ((skid_uchunkpg_t *)(((size_t)uchunk) & ~PGOFF_MAX))

// User chunk list
extern skid_uchunkpg_t *skid_uchunk_list;

#endif

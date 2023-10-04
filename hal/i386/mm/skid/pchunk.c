#include "../skid.h"

skid_pchunkpg_t *skid_pchunk_list = NULL;

/// @brief Create a new PChunk page.
///
/// @return Pointer to the new page, NULL if failed.
skid_pchunkpg_t *skid_pchunkpg_new() {
	skid_pchunkpg_t *vaddr = UNPGADDR(hn_kvpgalloc(mm_kernel_context->pdt));
	if (!vaddr)
		return NULL;

	void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, PAGE_READ | PAGE_WRITE, 0);
	if (!paddr) {
		mm_vmfree(mm_kernel_context, vaddr, sizeof(skid_pchunkpg_t));
		return NULL;
	}

	mm_mmap(mm_kernel_context, vaddr, paddr, sizeof(skid_pchunkpg_t), PAGE_READ | PAGE_WRITE);

	memset(vaddr, 0, sizeof(skid_pchunkpg_t));

	// Prepend to the list.
	if (skid_pchunk_list)
		skid_pchunk_list->last = vaddr;
	vaddr->next = skid_pchunk_list;
	skid_pchunk_list = vaddr;
	return vaddr;
}

/// @brief Free a PChunk page.
///
/// @param pg PChunk page to free.
void skid_pchunkpg_free(skid_pchunkpg_t *pg) {
	assert(skid_pchunkpg_isfree(pg));
	if (pg->last)
		pg->last->next = pg->next;
	else
		skid_pchunk_list = pg->next;
	if (pg->next)
		pg->next->last = pg->last;

	mm_pgfree(mm_getmap(mm_kernel_context, pg), 0);
	mm_vmfree(mm_kernel_context, pg, sizeof(skid_pchunk_t));
}

/// @brief Allocate a new PChunk slot.
///
/// @return Pointer to allocated slot, NULL if failed.
skid_pchunk_t *skid_pchunk_slotalloc() {
	skid_pchunkpg_foreach(i) {
		if (skid_pchunkpg_isfull(i))
			continue;
		skid_pchunk_foreach(i, j) {
			skid_pchunk_t *chunk = &(i->chunks[j]);
			if (!skid_pchunk_isvalid(chunk))
				return chunk;
		}
	}
	return NULL;
}

/// @brief Get PChunk which manages specified address.
///
/// @param ptr Address to find.
/// @return Found PChunk, NULL if not found.
skid_pchunk_t *skid_pchunk_get(const void *ptr, size_t size) {
	skid_pchunkpg_foreach(i) {
		if (skid_pchunkpg_isfree(i))
			continue;
		skid_pchunk_foreach(i, j) {
			skid_pchunk_t *chunk = &(i->chunks[j]);
			if (!skid_pchunk_isvalid(chunk))
				continue;
			if (ISOVERLAPPED(chunk->ptr, MM_BLKSIZE(chunk->order), ptr, size))
				return chunk;
		}
	}
	return NULL;
}

skid_pchunk_t *skid_pchunk_alloc(uint8_t order) {
	skid_pchunk_t *pchunk = skid_pchunk_slotalloc();

	if (pchunk) {
		if (!(pchunk->ptr = mm_pgalloc(MM_PMEM_AVAILABLE, PAGE_READ | PAGE_WRITE, order)))
			return NULL;
		pchunk->order = order;
		skid_pchunkpg_of(pchunk)->inuse_num++;
	} else {
		skid_pchunkpg_t *pg = skid_pchunkpg_new();
		assert(pg);

		pchunk = &pg->chunks[0];

		if (!(pchunk->ptr = mm_pgalloc(MM_PMEM_AVAILABLE, PAGE_READ | PAGE_WRITE, order))) {
			skid_pchunkpg_free(pg);
			return NULL;
		}

		pchunk->order = order;
		pg->inuse_num++;
	}

	return pchunk;
}

void skid_pchunk_incref(skid_pchunk_t *chunk) {
	assert(chunk);
	chunk->ref_num++;
}

void skid_pchunk_decref(skid_pchunk_t *chunk) {
	assert(chunk);
	if (!--chunk->ref_num) {
		mm_pgfree(chunk->ptr, chunk->order), chunk->ptr = NULL;
		skid_pchunkpg_t *pg = skid_pchunkpg_of(chunk);
		if (!--(pg->inuse_num))
			skid_pchunkpg_free(pg);
		else
			chunk->ptr = NULL, chunk->order = 0, chunk->ref_num = 0;
	}
}

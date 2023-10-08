#include "../skid.h"

skid_uchunkpg_t *skid_uchunk_list = NULL;

/// @brief Create a new UChunk page.
///
/// @return Pointer to the new page, NULL if failed.
skid_uchunkpg_t *skid_uchunkpg_new() {
	skid_uchunkpg_t *vaddr = UNPGADDR(hn_kvpgalloc(mm_kernel_context->pdt));
	if (!vaddr)
		return NULL;

	void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, 0);
	if (!paddr) {
		mm_vmfree(mm_kernel_context, vaddr, sizeof(skid_uchunkpg_t));
		return NULL;
	}

	mm_mmap(mm_kernel_context, vaddr, paddr, sizeof(skid_uchunkpg_t), PAGE_READ | PAGE_WRITE);

	memset(vaddr, 0, sizeof(skid_uchunkpg_t));

	// Prepend to the list.
	if (skid_uchunk_list)
		skid_uchunk_list->last = vaddr;
	vaddr->next = skid_uchunk_list;
	skid_uchunk_list = vaddr;
	return vaddr;
}

/// @brief Free a UChunk page.
///
/// @param pg UChunk page to free.
void skid_uchunkpg_free(skid_uchunkpg_t *pg) {
	assert(skid_uchunkpg_isfree(pg));
	if (pg->last)
		pg->last->next = pg->next;
	else
		skid_uchunk_list = pg->next;
	if (pg->next)
		pg->next->last = pg->last;

	mm_pgfree(mm_getmap(mm_kernel_context, pg), 0);
	mm_vmfree(mm_kernel_context, pg, sizeof(skid_uchunk_t));
}

/// @brief Allocate a new UChunk slot.
///
/// @return Pointer to allocated slot, NULL if failed.
skid_uchunk_t *skid_uchunk_slotalloc() {
	skid_uchunkpg_foreach(i) {
		if (skid_uchunkpg_isfull(i))
			continue;
		skid_uchunk_foreach(i, j) {
			skid_uchunk_t *chunk = &(i->chunks[j]);
			if (!skid_uchunk_isvalid(chunk))
				return chunk;
		}
	}
	return NULL;
}

/// @brief Get UChunk which manages specified address.
///
/// @param ptr Address to find.
/// @return Found UChunk, NULL if not found.
skid_uchunk_t *skid_uchunk_get(const void *ptr, size_t size) {
	skid_uchunkpg_foreach(i) {
		if (skid_uchunkpg_isfree(i))
			continue;
		skid_uchunk_foreach(i, j) {
			skid_uchunk_t *chunk = &(i->chunks[j]);
			if (!skid_uchunk_isvalid(chunk))
				continue;
			if (ISOVERLAPPED(chunk->ptr, chunk->size, ptr, size))
				return chunk;
		}
	}
	return NULL;
}

skid_uchunk_t *skid_uchunk_get_exact(const void *ptr) {
	skid_uchunkpg_foreach(i) {
		if (skid_uchunkpg_isfree(i))
			continue;
		skid_uchunk_foreach(i, j) {
			skid_uchunk_t *chunk = &(i->chunks[j]);
			if (!skid_uchunk_isvalid(chunk))
				continue;
			if (chunk->ptr == ptr)
				return chunk;
		}
	}
	return NULL;
}

skid_uchunk_t *skid_uchunk_alloc(void *ptr, size_t size) {
	skid_uchunk_t *uchunk = skid_uchunk_slotalloc();

	if (!uchunk) {
		skid_uchunkpg_t *pg = skid_uchunkpg_new();
		assert(pg);

		uchunk = &pg->chunks[0];
	}

	uchunk->ptr = ptr;
	uchunk->size = size;

	skid_uchunkpg_of(uchunk)->inuse_num++;
	return uchunk;
}

void skid_uchunk_free(skid_uchunk_t *uchunk) {
	assert(uchunk);
	uchunk->ptr = NULL;
	uchunk->size = 0;

	if (!--(skid_uchunkpg_of(uchunk)->inuse_num))
		skid_uchunkpg_free(skid_uchunkpg_of(uchunk));
}

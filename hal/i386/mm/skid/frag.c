#include "../skid.h"

skid_fragpg_t *skid_frag_list = NULL;

/// @brief Create a new UChunk page and prepend it to the list.
///
/// @param paddrDest Where to store its physical address, NULL if unneeded.
/// @return Created UChunk page.
skid_fragpg_t *skid_fragpg_new() {
	skid_fragpg_t *vaddr = (skid_fragpg_t *)UNPGADDR(hn_kvpgalloc(mm_kernel_context->pdt));
	if (!vaddr)
		return NULL;

	void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, 0);
	if (!paddr) {
		mm_vmfree(mm_kernel_context, (void *)vaddr, sizeof(skid_fragpg_t));
		return NULL;
	}

	mm_mmap(mm_kernel_context, (void *)vaddr, paddr, sizeof(skid_fragpg_t), PAGE_READ | PAGE_WRITE);

	memset((void *)vaddr, 0, sizeof(skid_fragpg_t));

	// Prepend to the list.
	if (skid_frag_list)
		skid_frag_list->last = vaddr;
	vaddr->next = skid_frag_list;
	skid_frag_list = vaddr;

	return vaddr;
}

/// @brief Free a UChunk page and remove it from the list.
///
/// @param pg UChunk page to be freed.
void skid_fragpg_free(skid_fragpg_t *pg) {
	assert(skid_fragpg_isfree(pg));

	if (pg->last)
		pg->last->next = pg->next;
	else
		skid_frag_list = pg->next;
	if (pg->next)
		pg->next->last = pg->last;

	mm_pgfree(mm_getmap(mm_kernel_context, pg), 0);
	mm_vmfree(mm_kernel_context, pg, sizeof(skid_fragpg_t));
}

/// @brief Allocate a UChunk slot from pages in the list.
///
/// @return Allocated slot, NULL if no free slot.
skid_frag_t *skid_frag_slotalloc() {
	skid_fragpg_foreach(i) {
		if (skid_fragpg_isfull(i))
			continue;
		skid_frag_foreach(i, j) {
			skid_frag_t *cur_desc = &(i->entries[j]);
			if (!skid_frag_isvalid(cur_desc))
				return cur_desc;
		}
	}
	return NULL;
}

/// @brief Get a UChunk by an address exactly.
///
/// @param ptr Exact address to find.
/// @return Found UChunk, NULL if not found.
skid_frag_t *skid_frag_get_exact(const void *ptr) {
	skid_fragpg_foreach(i) {
		if (skid_fragpg_isfree(i))
			continue;
		skid_frag_foreach(i, j) {
			skid_frag_t *chunk = &(i->entries[j]);
			if (chunk->vaddr == ptr)
				return chunk;
		}
	}
	return NULL;
}

/// @brief Get UChunk which manages specified address.
///
/// @param ptr Address to find.
/// @return Found UChunk, NULL if not found.
skid_frag_t *skid_frag_get(const void *ptr, size_t size) {
	skid_fragpg_foreach(i) {
		if (skid_fragpg_isfree(i))
			continue;
		skid_frag_foreach(i, j) {
			skid_frag_t *chunk = &(i->entries[j]);
			if (skid_frag_isvalid(chunk)) {
				if (ISOVERLAPPED((size_t)chunk->vaddr, chunk->size, (size_t)ptr, size))
					return chunk;
			}
		}
	}
	return NULL;
}

/// @brief Get a UChunk by specified physical address.
///
/// @param ptr Physical address to find
/// @return Found UChunk, NULL if not found.
skid_frag_t *skid_frag_get_by_paddr(const void *ptr, size_t size) {
	skid_fragpg_foreach(i) {
		if (skid_fragpg_isfree(i))
			continue;
		skid_frag_foreach(i, j) {
			skid_frag_t *chunk = &(i->entries[j]);
			if (chunk->vaddr) {
				if (ISOVERLAPPED((size_t)chunk->paddr, chunk->size, (size_t)ptr, size))
					return chunk;
			}
		}
	}
	return NULL;
}

skid_frag_t *skid_frag_alloc(const void *vaddr, const void *paddr, size_t size) {
	skid_frag_t *frag = skid_frag_slotalloc();

	if (!frag) {
		// Create a new chunk page.
		skid_fragpg_t *newpg = skid_fragpg_new();
		assert(newpg);

		// Set the first chunk to input area.
		frag = &newpg->entries[0];
	}

	frag->vaddr = (void *)vaddr;
	frag->paddr = (void *)paddr;
	frag->size = size;
	skid_fragpg_of(frag)->inuse_num++;
	return frag;
}

void skid_frag_free(skid_frag_t *frag) {
}

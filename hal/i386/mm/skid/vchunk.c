#include "../skid.h"

skid_vchunkpg_t *skid_vchunk_list = NULL;

/// @brief Create a new VChunk page.
///
/// @return Pointer to the new page, NULL if failed.
skid_vchunkpg_t *skid_vchunkpg_new() {
	skid_vchunkpg_t *vaddr = UNPGADDR(hn_kvpgalloc(mm_kernel_context->pdt));
	if (!vaddr)
		return NULL;

	void *paddr = mm_pgalloc(MM_PMEM_AVAILABLE, 0);
	if (!paddr) {
		mm_vmfree(mm_kernel_context, (void *)vaddr, sizeof(skid_vchunkpg_t));
		return NULL;
	}

	mm_mmap(mm_kernel_context, (void *)vaddr, paddr, sizeof(skid_vchunkpg_t), PAGE_READ | PAGE_WRITE);

	memset((void *)vaddr, 0, sizeof(skid_vchunkpg_t));

	// Prepend to the list.
	if (skid_vchunk_list)
		skid_vchunk_list->last = vaddr;
	vaddr->next = skid_vchunk_list;
	skid_vchunk_list = vaddr;
	return vaddr;
}

/// @brief Free a VChunk page.
///
/// @param pg Pointer to VChunk to be free.
void skid_vchunkpg_free(skid_vchunkpg_t *pg) {
	assert(skid_vchunkpg_isfree(pg));

	if (pg->last)
		pg->last->next = pg->next;
	else
		skid_vchunk_list = pg->next;
	if (pg->next)
		pg->next->last = pg->last;

	mm_pgfree(mm_getmap(mm_kernel_context, pg), 0);
	mm_vmfree(mm_kernel_context, pg, sizeof(skid_vchunkpg_t));
}

/// @brief Allocate a new VChunk slot.
///
/// @return Pointer to allocated slot, NULL if failed.
skid_vchunk_t *skid_vchunk_slotalloc() {
	skid_vchunkpg_foreach(i) {
		for (uint16_t j = 0; j < ARRAYLEN(i->chunks); ++j) {
			if (!skid_vchunk_isvalid(&(i->chunks[j])))
				return i->chunks[j].ptr;
		}
	}
	return NULL;
}

/// @brief Allocate a new VChunk with a virtual memory area.
///
/// @param size Size of the new area.
/// @return Pointer to the VChunk, NULL if failed.
skid_vchunk_t *skid_vchunk_alloc(size_t size) {
	skid_vchunk_t *chunk = skid_vchunk_slotalloc();
	if (!chunk) {
		skid_vchunkpg_t *pg = skid_vchunkpg_new();
		assert(pg);
		chunk = &pg->chunks[0];
	}

	chunk->ptr = mm_kvmalloc(mm_kernel_context, size, PAGE_READ | PAGE_WRITE);
	assert(chunk->ptr);

	chunk->pg_num = PGROUNDUP(size);
	chunk->ref_num = 0;

	skid_vchunkpg_of(chunk)->inuse_num++;

	return chunk;
}

/// @brief Get VChunk which manages specified address.
///
/// @param ptr Address to find.
/// @return Found VChunk, NULL if not found.
skid_vchunk_t *skid_vchunk_get(const void *ptr, size_t size) {
	skid_vchunkpg_foreach(i) {
		if (!i->inuse_num)
			continue;
		skid_vchunk_foreach(i, j) {
			skid_vchunk_t *chunk = &(i->chunks[j]);
			if (!skid_vchunk_isvalid(chunk))
				continue;
			if (ISOVERLAPPED(chunk->ptr, UNPGSIZE(chunk->pg_num), ptr, size))
				return chunk;
		}
	}
	return NULL;
}

/// @brief Scan and merge continuous VChunks.
void skid_vchunk_scan_and_merge() {
	skid_vchunkpg_foreach(i) {
		if (skid_vchunkpg_isfree(i))
			continue;
		skid_vchunk_foreach(i, j) {
			skid_vchunk_t *chunk = &(i->chunks[j]);
			if (!skid_vchunk_isvalid(chunk))
				continue;
			skid_vchunk_t *nearest_chunk = skid_vchunk_get(chunk->ptr + UNPGSIZE(chunk->pg_num), 0);
			if (nearest_chunk) {
				chunk->pg_num += nearest_chunk->pg_num;
				chunk->ref_num += nearest_chunk->ref_num;

				// Free the merged chunk.
				nearest_chunk->ptr = NULL;
				nearest_chunk->pg_num = 0;
				nearest_chunk->ref_num = 0;
				if (!--(skid_vchunkpg_of(nearest_chunk)->inuse_num))
					skid_vchunkpg_free(skid_vchunkpg_of(nearest_chunk));
			}
		}
	}
}

void skid_vchunk_incref(skid_vchunk_t *chunk) {
	assert(chunk);
	chunk->ref_num++;
}

void skid_vchunk_decref(skid_vchunk_t *chunk) {
	assert(chunk);
	if (!--chunk->ref_num) {
		mm_vmfree(mm_kernel_context, chunk->ptr, UNPGSIZE(chunk->pg_num));

		chunk->ptr = NULL;
		chunk->pg_num = 0;

		if (!--(skid_vchunkpg_of(chunk)->inuse_num))
			skid_vchunkpg_free(skid_vchunkpg_of(chunk));
	}
}

#include <oicos/km/logger.h>
#include "../skid.h"

void *mm_kmalloc(size_t size) {
	size = SIZE_CEIL(size);

	if (!size)
		return NULL;

	size_t sz_left = size;
	void *const vaddr = skid_vmalloc(size), *vaddr_cur = vaddr;
	assert(vaddr);

	skid_uchunk_t *uchunk = skid_uchunk_alloc(vaddr, size);

	// Allocate from existing PChunks.
	if (size < PAGESIZE) {
		// +----------+---------|-----+
		// | Previous | Current |     | ...
		// +----------+---------|-----+
		//                      ^ Page border
		// Allocate in the gap between the first allocated page and the next page.
		if (((size_t)vaddr) & PGOFF_MAX) {
			const size_t sz_alloc = MIN(PAGESIZE - (((size_t)vaddr) & PGOFF_MAX), sz_left);
			void *const paddr = mm_getmap(mm_kernel_context, vaddr);
			assert(paddr);

			skid_frag_alloc(vaddr, paddr, sz_alloc);

			skid_pchunk_t *pchunk = skid_pchunk_get(paddr, sz_alloc);
			assert(pchunk);
			skid_pchunk_incref(pchunk);

			sz_left -= sz_alloc;
			vaddr_cur += sz_alloc;
		}

		// We assume that the size is smaller than a whole page.
		skid_pchunkpg_foreach(i) {
			skid_pchunk_foreach(i, j) {
				skid_pchunk_t *const pchunk = &i->chunks[j];

				if (!skid_pchunk_isvalid(pchunk))
					continue;

				assert(pchunk->ptr > (void *)KERNEL_PTOP);

				if (!sz_left)
					return vaddr;

				if (sz_left < size) {
					// Tail
					for (pgsize_t blkoff = 0; blkoff < MM_PGUNWIND(pchunk->order, 1); blkoff++) {
						skid_frag_t *frag = skid_frag_get_by_paddr(pchunk->ptr + blkoff, sz_left);
						if (frag)
							continue;
						// Succeeded
						skid_frag_alloc(vaddr_cur, pchunk->ptr + blkoff, sz_left);
						mm_mmap(mm_kernel_context, vaddr_cur, pchunk->ptr + blkoff, sz_left, PAGE_READ | PAGE_WRITE);

						skid_pchunk_incref(pchunk);
						return vaddr;
					}
				} else {
					// Head
					for (pgsize_t blkoff = 0; blkoff < MM_PGUNWIND(pchunk->order, 1); blkoff++)
						for (size_t pgoff_diff = 0; pgoff_diff < PAGESIZE; pgoff_diff++) {
							void *paddr = pchunk->ptr + PAGESIZE * blkoff + pgoff_diff;
							const size_t sz_alloc = PAGESIZE - pgoff_diff;

							skid_frag_t *frag = skid_frag_get_by_paddr(paddr, sz_alloc);
							if (frag) {
								pgoff_diff += frag->size;
								continue;
							}

							// We only can allocate a single page for the head.
							skid_frag_alloc(vaddr_cur, paddr + pgoff_diff, sz_left);
							mm_mmap(mm_kernel_context, vaddr_cur, paddr, sz_left, PAGE_READ | PAGE_WRITE);

							skid_pchunk_incref(pchunk);
							vaddr_cur += sz_alloc, sz_left -= sz_alloc;
							goto minblk_succeed;
						}
				minblk_succeed:;
				}

				if (!sz_left)
					return vaddr;
			}
		}
	} else {
		// +----------+---------|-----+
		// | Previous | Current |     | ...
		// +----------+---------|-----+
		//                      ^ Page border
		// Allocate gap at the end of first allocated page.
		if (((size_t)vaddr) & PGOFF_MAX) {
			const size_t sz_alloc = PAGESIZE - (((size_t)vaddr) & PGOFF_MAX);
			void *const paddr = mm_getmap(mm_kernel_context, vaddr);
			assert(paddr);

			skid_frag_alloc(vaddr, paddr, sz_alloc);

			skid_pchunk_t *pchunk = skid_pchunk_get(paddr, sz_alloc);
			assert(pchunk);
			skid_pchunk_incref(pchunk);

			sz_left -= sz_alloc;
			vaddr_cur += sz_alloc;
		}

		skid_pchunkpg_foreach(i) {
			skid_pchunk_foreach(i, j) {
				skid_pchunk_t *pchunk = &i->chunks[j];

				if (!skid_pchunk_isvalid(pchunk))
					continue;

				if (sz_left <= PAGESIZE) {
					// Tail
					// Must be on the beginning of the page.
					for (pgsize_t blkoff = 0; blkoff < MM_PGUNWIND(pchunk->order, 1); blkoff++) {
						skid_frag_t *frag = skid_frag_get_by_paddr(pchunk->ptr + blkoff, sz_left);
						if (frag)
							continue;
						// Succeeded
						skid_frag_alloc(vaddr_cur, pchunk->ptr + blkoff, sz_left);
						mm_mmap(mm_kernel_context, vaddr_cur, pchunk->ptr + blkoff, sz_left, PAGE_READ | PAGE_WRITE);

						skid_pchunk_incref(pchunk);
						return vaddr;
					}
				} else if (sz_left < size) {
					// Body
					// Must use the whole page.
					for (pgsize_t blkoff = 0; blkoff < MM_PGUNWIND(pchunk->order, 1); blkoff++) {
						skid_frag_t *frag = skid_frag_get_by_paddr(pchunk->ptr + blkoff, PAGESIZE);
						if (frag)
							continue;
						// Succeeded
						skid_frag_alloc(vaddr_cur, pchunk->ptr + blkoff, sz_left);
						mm_mmap(mm_kernel_context, vaddr_cur, pchunk->ptr + blkoff, sz_left, PAGE_READ | PAGE_WRITE);

						skid_pchunk_incref(pchunk);
						vaddr_cur += PAGESIZE, sz_left -= PAGESIZE;
						break;
					}
				} else {
					// Head
					// Must be on the ending of the page.
					// Assumed that the size is greater than a whole page.
					for (pgsize_t blkoff = 0; blkoff < MM_PGUNWIND(pchunk->order, 1); blkoff++)
						for (size_t pgoff_diff = 0; pgoff_diff < PAGESIZE; pgoff_diff++) {
							void *paddr = pchunk->ptr + PAGESIZE * blkoff + pgoff_diff;
							const size_t sz_alloc = PAGESIZE - pgoff_diff;

							skid_frag_t *frag = skid_frag_get_by_paddr(paddr, sz_alloc);
							if (frag) {
								pgoff_diff += frag->size;
								continue;
							}

							// We only can allocate a single page for the head.
							skid_frag_alloc(vaddr_cur, paddr + pgoff_diff, sz_left);
							mm_mmap(mm_kernel_context, vaddr_cur, paddr, sz_left, PAGE_READ | PAGE_WRITE);

							skid_pchunk_incref(pchunk);
							vaddr_cur += sz_alloc, sz_left -= sz_alloc;
							goto majblk_succeed;
						}
				majblk_succeed:;
				}

				if (!sz_left)
					return vaddr;
			}
		}
	}

	// Allocate by creating new PChunks if there's no any free existing PChunk.
	while (sz_left) {
		// Order of block to allocate.
		uint16_t order = hn_get_alloc_order(sz_left);

		// Size of fragment to allocate.
		const size_t sz_alloc = MIN(MM_BLKSIZE(order), sz_left);

		skid_pchunk_t *const pchunk = skid_pchunk_alloc(order);
		assert(pchunk);

		// Map the allocated block.
		mm_mmap(mm_kernel_context, vaddr_cur, pchunk->ptr, sz_alloc, PAGE_READ | PAGE_WRITE);

		skid_frag_alloc(vaddr_cur, pchunk->ptr, sz_alloc);
		skid_pchunk_incref(pchunk);

		sz_left -= sz_alloc;
		vaddr_cur += sz_alloc;
	}

	return vaddr;
}

void mm_kfree(void *ptr) {
	skid_uchunk_t *uchunk = skid_uchunk_get_exact(ptr);
	if (!uchunk)
		km_panic("No kernel chunk at %p", ptr);

	void *addr_cur = uchunk->ptr;
	size_t sz_cur = uchunk->size;

	// Dereference for each VChunk referenced by the UChunk.
	{
		skid_vchunk_t *i;

		addr_cur = uchunk->ptr;
		sz_cur = uchunk->size;

		while (sz_cur) {
			i = skid_vchunk_get(addr_cur, sz_cur);
			assert(i);
			addr_cur += UNPGSIZE(i->pg_num), sz_cur -= MIN(UNPGSIZE(i->pg_num), sz_cur);
			skid_vchunk_decref(i);
		}
	}

	// Dereference for each fragment referenced by the UChunk.
	{
		skid_frag_t *i;

		addr_cur = uchunk->ptr;
		sz_cur = uchunk->size;

		while (sz_cur) {
			i = skid_frag_get(addr_cur, sz_cur);
			assert(i);

			skid_pchunk_t *pchunk = skid_pchunk_get(i->paddr, i->size);
			assert(pchunk);
			skid_pchunk_decref(pchunk);

			addr_cur += i->size, sz_cur -= MIN(i->size, sz_cur);
			i->paddr = NULL, i->vaddr = NULL, i->size = 0;
			if (!--(skid_fragpg_of(i)->inuse_num))
				skid_fragpg_free(skid_fragpg_of(i));
		}
	}

	skid_uchunk_free(uchunk);
}

/// @brief Allocate a continuous virtual memory space.
/// @note SKID may map free VChunks to physical address 0x00000000 and that is not a bug.
///
/// @param size Size to allocate.
/// @return Pointer to allocated virtual memory space.
void *skid_vmalloc(size_t size) {
	// Allocate from existing VChunks.
	skid_vchunkpg_foreach(i) {
		skid_vchunk_foreach(i, j) {
			skid_vchunk_t *vchunk = &i->chunks[j];
			if (!skid_vchunk_isvalid(vchunk))
				continue;

			size_t chunk_off_max = UNPGSIZE(vchunk->pg_num) - size;

			for (size_t chunk_off = 0; chunk_off_max;) {
				void *ptr = vchunk->ptr + chunk_off;
				skid_uchunk_t *uchunk = skid_uchunk_get(ptr, size);
				if (uchunk) {
					chunk_off += uchunk->size;
					continue;
				}
				skid_vchunk_incref(vchunk);
				return ptr;
			}
		}
	}

	// Allocate a new VChunk.
	skid_vchunk_t *newpage = skid_vchunk_alloc(size);
	skid_vchunk_scan_and_merge();
	return skid_vmalloc(size);
}

#include <pbos/km/logger.h>

#include "../skid.h"

void skid_dbg_dump_frags() {
	skid_fragpg_foreach(i) {
		if (skid_fragpg_isfree(i))
			continue;
		skid_frag_foreach(i, j) {
			skid_frag_t *chunk = &(i->entries[j - 1]);
			if (skid_frag_isvalid(chunk))
				kprintf("Frag %p[%d]: vaddr=%p, paddr=%p, sz=%d\n", i, (int)j, chunk->vaddr, chunk->paddr, (int)chunk->size);
		}
	}
}

void skid_dbg_dump_uchunks() {
	skid_uchunkpg_foreach(i) {
		if (skid_uchunkpg_isfree(i))
			continue;
		skid_uchunk_foreach(i, j) {
			skid_uchunk_t *chunk = &i->chunks[j - 1];
			if (skid_uchunk_isvalid(chunk))
				kprintf("UChunk %p[%d]: ptr=%p, sz=%d\n", i, (int)j, chunk->ptr, (int)chunk->size);
		}
	}
}

void skid_dbg_dump_vchunks() {
	skid_vchunkpg_foreach(i) {
		if (skid_vchunkpg_isfree(i))
			continue;
		skid_vchunk_foreach(i, j) {
			skid_vchunk_t *chunk = &i->chunks[j - 1];
			if (skid_vchunk_isvalid(chunk))
				kprintf("VChunk %p[%d]: ptr=%p, sz=%d\n", i, (int)j, chunk->ptr, (int)UNPGSIZE(chunk->pg_num));
		}
	}
}

void skid_dbg_dump_pchunks() {
	skid_pchunkpg_foreach(i) {
		if (skid_pchunkpg_isfree(i))
			continue;
		skid_pchunk_foreach(i, j) {
			skid_pchunk_t *chunk = &i->chunks[j - 1];
			if (skid_pchunk_isvalid(chunk))
				kprintf("PChunk %p[%d]: ptr=%p, sz=%d, ref_num=%d\n", i, (int)j, chunk->ptr, MM_BLKSIZE(chunk->order), (int)chunk->ref_num);
		}
	}
}

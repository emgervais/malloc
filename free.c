#include "malloc.h"

void free(void *ptr) {
    if (!ptr)
        return;
    chunk *current = (chunk *)((char *)ptr - CHUNK_HEADER_SIZE);
    if ((current->size & 0x2)) {
        large_block *tmp = g_arena.large_blocks;
        if (tmp && (char *)&tmp->data == (char *)ptr) {
            g_arena.large_blocks = tmp->next;
            munmap(tmp, current->size & ~0x7);
            return;
        }
        
        while (tmp && tmp->next) {
            if (tmp->next->data == (char *)ptr) {
                large_block *to_remove = tmp->next;
                size_t s = to_remove->size & ~0x7;
                tmp->next = to_remove->next;
                munmap(to_remove, s);
                return;
            }
            tmp = tmp->next;
        }
    }
    size_t size = current->size & ~0x7;
    chunk *next_physical = (chunk *)((char *)current + size);
    zone *z = size - CHUNK_HEADER_SIZE <= MAX_TINY_SIZE ? g_arena.tiny_zones : g_arena.small_zones;
    if (next_physical == z->top) {
        z->top = current;
        return;
    }
    current->fwd = NULL;
    current->bck = NULL;
    if (next_physical < z->top) {
        next_physical->size &= ~0x1;
    }
    if (!z->bin) {
        z->bin = current;
    } else {
        chunk *tmp_chunk = z->bin;
        while (tmp_chunk->fwd) {
            tmp_chunk = tmp_chunk->fwd;
        }
        tmp_chunk->fwd = current;
        current->bck = tmp_chunk;
    }
}

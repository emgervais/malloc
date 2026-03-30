#include "malloc.h"

void *fresh_ralloc(void *old, size_t size, size_t old_user_size) {
    void *new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    
    size_t copy_size = old_user_size < size ? old_user_size : size;
    for (size_t i = 0; i < copy_size; i++) {
        ((char *)new_ptr)[i] = ((char *)old)[i];
    }
    free(old);
    return new_ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    if (size <= 0) {
        free(ptr);
        return NULL;
    }
    
    chunk *current = (chunk *)((char *)ptr - CHUNK_HEADER_SIZE);
    size_t curr_size = current->size & ~0x7;
    size_t user_size = curr_size - CHUNK_HEADER_SIZE;
    
    size_t new_chunk_size = (size + CHUNK_HEADER_SIZE + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);
    if (new_chunk_size < MIN_CHUNK_SIZE) {
        new_chunk_size = MIN_CHUNK_SIZE;
    }
    
    if (new_chunk_size <= curr_size) {
        // split if too big
        return ptr;
    }
    
    int was_tiny = (user_size <= MAX_TINY_SIZE);
    int is_now_tiny = (size <= MAX_TINY_SIZE);
    
    int was_small = (user_size > MAX_TINY_SIZE && user_size <= MIN_LARGE_SIZE);
    int is_now_small = (size > MAX_TINY_SIZE && size <= MIN_LARGE_SIZE);

    int is_now_large = (size > MIN_LARGE_SIZE);

    if ((was_tiny != is_now_tiny) || (was_small != is_now_small) || is_now_large) {
        return fresh_ralloc(ptr, size, user_size);
    }
    
    zone *z = is_now_tiny ? g_arena.tiny_zones : g_arena.small_zones;
    size_t total_size = curr_size;
    chunk *scan_chunk = (chunk *)((char *)current + curr_size);
    
    while (total_size < new_chunk_size) {
        if (scan_chunk == z->top) {
            size_t needed = new_chunk_size - total_size;
            if ((char *)z->top + needed <= (char *)z->end) {
                total_size = new_chunk_size;
            }
            break;
        }
        
        size_t scan_size = scan_chunk->size & ~0x7;
        chunk *scan_next = (chunk *)((char *)scan_chunk + scan_size);
        
        if (scan_next == z->top || (scan_next->size & 0x1)) {
            break; 
        }
        
        total_size += scan_size;
        scan_chunk = scan_next;
    }
    
    if (total_size >= new_chunk_size) {
        size_t absorbed_size = curr_size;
        chunk *merge_chunk = (chunk *)((char *)current + curr_size);
        
        while (absorbed_size < new_chunk_size) {
            if (merge_chunk == z->top) {
                size_t needed = new_chunk_size - absorbed_size;
                z->top = (chunk *)((char *)z->top + needed);
                current->size = new_chunk_size | (current->size & 0x7);
                return ptr;
            }
            
            size_t m_size = merge_chunk->size & ~0x7;
            chunk *m_next = (chunk *)((char *)merge_chunk + m_size);
            
            if (!merge_chunk->bck && !merge_chunk->fwd) {
                z->bin = NULL;
            } else if (!merge_chunk->bck) {
                z->bin = merge_chunk->fwd;
                merge_chunk->fwd->bck = NULL;
            } else if (!merge_chunk->fwd) {
                merge_chunk->bck->fwd = NULL;
            } else {
                merge_chunk->bck->fwd = merge_chunk->fwd;
                merge_chunk->fwd->bck = merge_chunk->bck;
            }
            
            absorbed_size += m_size;
            merge_chunk = m_next;
        }
        
        current->size = absorbed_size | (current->size & 0x7);
        
        if (merge_chunk < z->top) {
            merge_chunk->size |= 0x1;
        }
        
        return ptr;
    }

    return fresh_ralloc(ptr, size, user_size);
}

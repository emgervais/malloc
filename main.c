#include "malloc.h"

arena g_arena;

zone *init_zone(size_t block_size, size_t blocks_nbr) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t zone_size = blocks_nbr * (block_size + sizeof(zone));
    size_t mmap_size = (zone_size % page_size != 0 ? zone_size / page_size + 1 : zone_size / page_size) * page_size;
    zone *zone_addr = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (DEBUG) {
        write(1, "New zone(page) of size ", 24);
        putnbr(mmap_size);
        write(1, "\n", 1);
    }
    if (zone_addr == (zone *) -1) return NULL;
    
    zone_addr->block_size = block_size;
    zone_addr->next = NULL;
    zone_addr->top = (chunk *)((char *)zone_addr + sizeof(zone));
    zone_addr->bin = NULL;
    zone_addr->end = (char *)zone_addr + mmap_size;
    return zone_addr;
}
void init_heap() {
    g_arena.tiny_max = 128;
    g_arena.small_max = 1024;
    g_arena.blocks_nbr = 100;
    g_arena.large_blocks = NULL;

    g_arena.tiny_zones = NULL; //init_zone(g_arena.tiny_max, g_arena.blocks_nbr);
    g_arena.small_zones = NULL; //init_zone(g_arena.small_max, g_arena.blocks_nbr);
}
void *large_alloc(size_t size) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    size += sizeof(large_block);
    size_t mmap_size = (size + page_size - 1) & ~(page_size - 1);
    large_block *block = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (DEBUG) {
        printf("Large alloc at %p of size %ld requested size + header %ld\n", block, mmap_size, size);
    }
    if (block == (large_block *) -1) return NULL;
    block->next = NULL;
    block->size = mmap_size | 0x2;
    large_block *temp = g_arena.large_blocks;
    if (!temp) {
        g_arena.large_blocks = block;
        return block->data;
    }
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = block;
    return block->data;
}

void fill_chunk(zone *z, chunk *current, size_t size) {
    (void) size;
    size_t current_size = current->size & ~0x7;
    // size_t flags = current->size & 0x7;
    // split if too big
    chunk *next_physical = (chunk *)((char *)current + current_size);
    next_physical->size |= 0x1;
    if (!current->bck && !current->fwd) {
        z->bin = NULL;
    } else if (!current->bck) {
        current->fwd->bck = NULL;
    } else if (!current->fwd) {
        current->bck->fwd = NULL;
    } else {
        current->bck->fwd = current->fwd;
        current->fwd->bck = current->bck;
    }
}
void *alloc(size_t size, zone *z) {
    zone *temp = z;
    while (temp) {
        chunk *current = temp->bin;
        while (current) {
            // is chunk good?
            if (current->size - CHUNK_HEADER_SIZE >= size) {
                fill_chunk(temp, current, size);
                return &current->data;
            }
            current = current->fwd;
        }
        if (!current) {
            if (DEBUG) {
                write(1, "No fitting chunk in bin\n", 25);
            }
            size_t chunk_size = (size + CHUNK_HEADER_SIZE + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);
            if (chunk_size < MIN_CHUNK_SIZE) {
                chunk_size = MIN_CHUNK_SIZE;
            }
            if ((char *)temp->top + chunk_size <= (char *)temp->end) {
                chunk *new_chunk = temp->top;
                new_chunk->prev_size = 0;
                new_chunk->size = chunk_size | 0x1;
                temp->top = (chunk *)((char *)temp->top + chunk_size);
                temp->top->prev_size = chunk_size;
                
                return new_chunk->data;
            }
            if (DEBUG) {
                write(1, "New chunk couldnt fit in current zone\n", 39);
            }
        }
        if (!temp->next)
            break;
        temp = temp->next;
    }
    if (DEBUG) {
        write(1, "Couldnt find a good zone, allocating a new one\n", 48);
    } 
    temp->next = init_zone(z->block_size, g_arena.blocks_nbr);
    return alloc(size, temp->next);
}

void *malloc(size_t size) {
    if (size <= 0) {
        return NULL;
    }
    if (!g_arena.tiny_zones && ! g_arena.small_zones && !g_arena.large_blocks) {
        init_heap();
    }
    if (size > MIN_LARGE_SIZE) {
        return large_alloc(size);
    }
    else {
        zone *z = size <= MAX_TINY_SIZE ? g_arena.tiny_zones : g_arena.small_zones;
        if (!z) {
            if (size <= MAX_TINY_SIZE) {
                g_arena.tiny_zones = init_zone(g_arena.tiny_max, g_arena.blocks_nbr);
                z = g_arena.tiny_zones ;
            } else {
                g_arena.small_zones = init_zone(g_arena.small_max, g_arena.blocks_nbr);
                z = g_arena.small_zones ;
            }
        }
        return alloc(size, z);
    }
}


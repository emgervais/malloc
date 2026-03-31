#include "malloc.h"

int is_chunk_free(chunk *target, chunk *bin_list) {
    chunk *curr = bin_list;
    while (curr) {
        if (curr == target) return 1;
        curr = curr->fwd;
    }
    return 0;
}

void show_alloc_mem() {
    size_t total_bytes = 0;
    
    zone *t_zone = g_arena.tiny_zones;
    while (t_zone) {
        putstr("TINY : ");
        print_address(t_zone);
        putstr("\n");
        
        chunk *current = (chunk *)((char *)t_zone + sizeof(zone));
        while (current < t_zone->top) {
            size_t size = current->size & ~0x7;


            if (!is_chunk_free(current, t_zone->bin)) {
                void *start_addr = current->data;
                void *end_addr = (char *)start_addr + (size - CHUNK_HEADER_SIZE);
                
                print_address(start_addr);
                putstr(" - ");
                print_address(end_addr);
                putstr(" : ");
                putnbr(size - CHUNK_HEADER_SIZE);
                putstr(" bytes\n");
                
                total_bytes += (size - CHUNK_HEADER_SIZE);
            }
            current = (chunk *)((char *)current + size);
        }
        t_zone = t_zone->next;
    }

    zone *s_zone = g_arena.small_zones;
    while (s_zone) {
        putstr("SMALL : ");
        print_address(s_zone);
        putstr("\n");
        
        chunk *current = (chunk *)((char *)s_zone + sizeof(zone));
        while (current < s_zone->top) {
            size_t size = current->size & ~0x7;

            if (!is_chunk_free(current, s_zone->bin)) {
                void *start_addr = current->data;
                void *end_addr = (char *)start_addr + (size - CHUNK_HEADER_SIZE);
                
                print_address(start_addr);
                putstr(" - ");
                print_address(end_addr);
                putstr(" : ");
                putnbr(size - CHUNK_HEADER_SIZE);
                putstr(" bytes\n");
                
                total_bytes += (size - CHUNK_HEADER_SIZE);
            }
            current = (chunk *)((char *)current + size);
        }
        s_zone = s_zone->next;
    }

    large_block *l_block = g_arena.large_blocks;
    if (l_block) {
        putstr("LARGE : ");
        print_address(l_block);
        putstr("\n");
        
        while (l_block) {
            void *start_addr = l_block->data;
            size_t actual_size = (l_block->size & ~0x7) - sizeof(large_block);
            void *end_addr = (char *)start_addr + actual_size;
            
            print_address(start_addr);
            putstr(" - ");
            print_address(end_addr);
            putstr(" : ");
            putnbr(actual_size);
            putstr(" bytes\n");
            
            total_bytes += actual_size;
            l_block = l_block->next;
        }
    }
    
    putstr("Total : ");
    putnbr(total_bytes);
    putstr(" bytes\n");
}

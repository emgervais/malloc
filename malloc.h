#ifndef MALLOC_H
#define MALLOC_H

#include <sys/mman.h>
#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

#define MAP_ANONYMOUS 0x20
#define MIN_LARGE_SIZE 1024
#define MAX_TINY_SIZE 128
#define MIN_CHUNK_SIZE 32
#define DEFAULT_MMAP_THRESHOLD 131072
#define CHUNK_HEADER_SIZE 2 * sizeof(size_t)
#define MEMORY_ALIGNMENT 16
#define BLOCK_NBR 100

typedef struct chunk {
  size_t prev_size;
  size_t size;
  union {
    char data[0];
    struct {
        struct chunk *fwd;
        struct chunk *bck;
    };
  };
} chunk;

typedef struct zone {
    size_t block_size;
    struct zone *next;
    chunk *bin;
    void *end;
    chunk *top;
    size_t padding; 
} zone;

typedef struct large_block {
    struct large_block *next;
    size_t size;
    char data[0];
} large_block;

typedef struct arena {
    int g_debug_mode;
    zone *tiny_zones;
    zone *small_zones;
    large_block *large_blocks;
    size_t tiny_max;
    size_t small_max;
    size_t blocks_nbr;
} arena;

extern arena g_arena;

void	putnbr(size_t nb);
void putstr(char *str);
void puthex(size_t nb);
void print_address(void *ptr);
int is_chunk_free(chunk *target, chunk *bin_list);
void show_alloc_mem();

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void hexdump_payload(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);

#endif

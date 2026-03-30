#include <sys/mman.h>
#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
  size_t prev_size;
  size_t size;
  char data[0];
} chunk;

int main() {
    void* a = mmap(NULL, 1000, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    chunk *b = a;
}
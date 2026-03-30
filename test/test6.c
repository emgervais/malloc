#include <stdlib.h>
void show_alloc_mem();
int main() { char *addr1; char *addr2; char *addr3; addr1 = (char*)malloc(10); addr2 = (char*)malloc(500); addr3 = (char*)malloc(5000); show_alloc_mem(); return 0; }

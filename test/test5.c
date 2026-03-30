#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main() { char *addr; char *addr2; addr = malloc(10); strcpy(addr, "Bonjour"); realloc(addr, 0); /* This should act like free(addr) */ addr2 = realloc(NULL, 20); strcpy(addr2, "World"); write(1, addr2, 6); write(1, "\n", 1); free(addr2); return 0; }

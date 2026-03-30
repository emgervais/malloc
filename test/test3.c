#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main() { char *addr1; char *addr2; addr1 = (char*)malloc(16 * 1024 * 1024); strcpy(addr1, "Bonjour"); write(1, addr1, 8); addr2 = (char*)realloc(addr1, 128 * 1024 * 1024); addr2[127 * 1024 * 1024] = 42; write(1, addr2, 8); write(1, "\n", 1); return 0; }

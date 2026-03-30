#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main() { char *addr; addr = malloc(16); strcpy(addr, "START"); addr = realloc(addr, 128); if (strcmp(addr, "START") == 0) { write(1, "OK\n", 3); } else { write(1, "KO\n", 3); } addr = realloc(addr, 4); if (strncmp(addr, "STAR", 4) == 0) { write(1, "OK\n", 3); } else { write(1, "KO\n", 3); } free(addr); return 0; }

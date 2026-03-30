#include <stdlib.h>
#include <string.h>
int main() { int i; char *addr; for (i = 0; i < 1024; i++) { addr = (char*)malloc(1024); addr[0] = 42; } return 0; }

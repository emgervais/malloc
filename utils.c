#include "malloc.h"

void	putnbr(size_t nb)
{
	if (nb <= 9) {
        char a[1] = {nb + 48};
		write(1, &a, 1);
    }
	else
	{
		putnbr(nb / 10);
		putnbr(nb % 10);
	}
}

void putstr(char *str) {
    size_t i = 0;
    while (str[i]) i++;
    write(1, str, i);
}

void puthex(size_t nb) {
    char base[] = "0123456789ABCDEF";
    if (nb >= 16) {
        puthex(nb / 16);
    }
    char c = base[nb % 16];
    write(1, &c, 1);
}

void print_address(void *ptr) {
    putstr("0x");
    puthex((size_t)ptr);
}

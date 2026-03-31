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

void puthex_byte(unsigned char byte) {
    char base[] = "0123456789ABCDEF";
    char c1 = base[byte / 16];
    char c2 = base[byte % 16];
    write(1, &c1, 1);
    write(1, &c2, 1);
}

void hexdump_payload(void *ptr, size_t size) {
    unsigned char *data = (unsigned char *)ptr;
    size_t i = 0;
    
    while (i < size) {

        size_t j = 0;
        while (j < 16 && i + j < size) {
            puthex_byte(data[i + j]);
            putstr(" ");
            if (j == 7) putstr(" ");
            j++;
        }

        

        j = 0;
        while (j < 16 && i + j < size) {
            unsigned char c = data[i + j];
            if (c >= 32 && c <= 126) {
                write(1, &c, 1);
            }
            j++;
        }
        
        putstr("\n");
        i += 16;
    }
}
ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so
SYMLINK = libft_malloc.so

SRCS = main.c utils.c free.c realloc.c show_alloc_mem.c
OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -shared $(CFLAGS) -o $@ $^
	ln -snf $@ $(SYMLINK)

%.o: %.c
	$(CC) $(CFLAGS) -Wno-error -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME) $(SYMLINK)

re: fclean all

.PHONY: all clean fclean re

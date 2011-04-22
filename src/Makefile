CC = clang
CFLAGS := -std=gnu99 -Wall -Wextra

SOURCES = \
	category.h category.c \
	data.h data.c \
	node.h node_eval.c node_print.c node_read.c \
	set.c set.h set_print.c \
	token.c token.h \
	uniset.c

uniset : $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

clean :
	rm -f uniset

.PHONY : clean

CC	=	gcc
SOURCE = $(wildcard *.c)
TARGETS = $(patsubst %.c, %, $(SOURCE))

CFLAGS = -Wall -g
 
all:$(TARGETS)

$(TARGETS):%:%.c
		$(CC) $< $(CFLAGS) -o $@

.PHONY:clean all
clean:
	rm -rf $(TARGETS) *.o

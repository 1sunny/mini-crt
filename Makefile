CC = gcc
CFLAGS = -fno-builtin -nostdlib -fno-stack-protector
OBJS = malloc.o printf.o stdio.o string.o

.PHONY: clean

all: minicrt.a test

%.o : %.c
	$(CC) -c $(CFLAGS) $<

minicrt.a: $(OBJS)
	ar -rs $@ $^

test: entry.o minicrt.a test.c
	$(CC) -ggdb -c $(CFLAGS) test.c -o test.o
	ld -static -e mini_crt_entry -o test entry.o test.o minicrt.a

clean:
	rm *.o *.a
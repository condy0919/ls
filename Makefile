.PHONY: all clean

CFLAGS=-std=c99 -g
CC=clang

all: ls.o
	$(CC) $(CFLAGS) ls.o -o ls -lm

ls.o:ls.h ls.c
	$(CC) ls.c -c $(CFLAGS)

clean:
	-rm ls.o

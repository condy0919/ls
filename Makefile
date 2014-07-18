.PHONY: all clean

CXXFLAGS=-std=c++11 -g -O2
CC=clang++

all: ls

ls: ls.o
	$(CC) ls.o -o ls

ls.o:

clean:
	$(RM) ls.o

.PHONY: all clean

all: ask2-fork fork-example

CC = gcc
CFLAGS = -g -Wall -O2 -std=c11
SHELL= /bin/bash

ask2-fork: proc-common.o ask2-fork.o
	$(CC) $(CFLAGS) $^ -o $@

fork-example: proc-common.o fork-example.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o tree-example fork-example pstree-this ask2-{fork,tree,signals,pipes}

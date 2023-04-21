.PHONY: all clean

all: ask2-tree

CC = gcc
CFLAGS = -g -Wall -O2 -std=c11
SHELL= /bin/bash

ask2-tree: proc-common.o ask2-tree.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o tree-example fork-example pstree-this ask2-{fork,tree,signals,pipes}

all: zing2 zing

zing2: zing2.o main.o
	gcc -Wall zing2.o main.o -o zing2

zing: zing.o main.o
	gcc -Wall zing.o main.o -o zing

zing2.o: zing2.c
	gcc -Wall -c zing2.c

main.o: main.c
	gcc -Wall -c main.c

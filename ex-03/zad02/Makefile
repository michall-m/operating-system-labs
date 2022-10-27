CC=gcc
CFLAGS=-Wall -std=c11

all: test

test: main
	./main < input.txt > raport2.txt

main: main.o liblib.a
	$(CC) $(CFLAGS) main.o liblib.a -o main

liblib.a: lib.o
	ar rcs liblib.a lib.o

lib.o: lib.c
	$(CC) $(CFLAGS) -c lib.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm *.o *.a main
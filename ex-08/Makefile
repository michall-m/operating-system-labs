all: clean main

main:
	gcc -Wall -o main main.c pgm_lib.c -lpthread

tests:
	./main 1 numbers body3.ascii.pgm negative_body3.ascii.pgm >	Times.txt
	./main 1 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 2 numbers body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 2 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 4 numbers body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 4 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 8 numbers body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 8 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 16 numbers body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 16 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 32 numbers body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt
	./main 32 block body3.ascii.pgm negative_body3.ascii.pgm >> Times.txt


clean:
	rm main
all: konsument producent pipe

konsument:
	gcc -Wall -std=c11 konsument.c -o konsument

producent:
	gcc -Wall -std=c11 producent.c -o producent

pipe:
	mkfifo PIPE

clean:
	rm -f konsument producent PIPE

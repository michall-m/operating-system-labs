run:
	mkfifo PIPE
	./producent PIPE 23 A.txt 5
	./producent PIPE 13 B.txt 5
	./konsument PIPE output.txt 5
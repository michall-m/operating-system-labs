#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Not enough args.");
        return -1;
    } else if (argc > 2) {
        printf("Too many args.");
        return -1;
    }
    if (argv[1][0] == '-'){
        printf("The number has to be positive.");
    }

    int nop;
    nop = atoi(argv[1]);

    for (int i = 0; i < nop; i++){
        if (fork() == 0){
            printf("i: %d: PID = %d\n", i, getpid());
            exit(0);
        }
    }
}
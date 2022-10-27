#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#define MAX_INDEX_LEN 16
#define SPECIAL_SIGN '$'
#define MAX_ROWS 256

int sshift(int rows_capacity[MAX_ROWS+1], int row){
    int result = 0;
    for(int i = 1; i <= row; i++){
        result += rows_capacity[i];
    }
    return result;
}

int main(int argc, char** argv){
    if(argc != 4){
        printf("Podaj 3 argumenty:\n");
        printf("- sciezka do potoku nazwanego,\n");
        printf("- sciezka do pliku tekstowego(zapis), \n");
        printf("- liczbe znakow odczytywanych jednorazowo z pliku.\n");
        return EXIT_FAILURE;
    }

    char* PIPE_PATH = argv[1];
    char* TXT_FILE_PATH = argv[2];
    const int CHARS_TO_READ = atoi(argv[3]);

    FILE* pipe = fopen(PIPE_PATH, "r");
    FILE* output = fopen(TXT_FILE_PATH, "w");
    int rows_capacity[MAX_ROWS + 1];
    for(int i = 0; i < MAX_ROWS + 1; i++){
        char n[1] = "\n";
        rows_capacity[i] = 1;
        fwrite(n, sizeof(char), 1, output);
    }
    rows_capacity[1] = 0;
    fseek(output, 0, SEEK_SET);

    char* text = malloc(sizeof(char) * CHARS_TO_READ);
    char* index = malloc(sizeof(char) * MAX_INDEX_LEN);
    char c;
    while(fread(&c, sizeof(char), 1, pipe) > 0){
        int size_counter = 0;
        if (c == SPECIAL_SIGN){
            fread(&c, sizeof(char), 1, pipe);
            while (c != SPECIAL_SIGN){
                index[size_counter++] = c;
                fread(&c, sizeof(char), 1, pipe);
            }
        }
        fread(text, sizeof(char), CHARS_TO_READ, pipe);
        int row = atoi(index);
        fseek(output, sshift(rows_capacity, row), SEEK_SET);
        rows_capacity[row] += CHARS_TO_READ;
        fwrite(text, sizeof(char), CHARS_TO_READ, output);
    }
}
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <math.h>


int main(int argc, char** argv){
    if(argc != 5){
        printf("Podaj 4 argumenty:\n");
        printf("- sciezka do potoku nazwanego,\n");
        printf("- numer wiersza,\n");
        printf("- sciezka do pliku tekstowego(odczyt), \n");
        printf("- liczbe znakow odczytywanych jednorazowo z pliku.\n");
        return EXIT_FAILURE;
    }

    const char* PIPE_PATH = argv[1];
    const int ROW_INDEX = atoi(argv[2]);
    const int ROW_INDEX_SIZE = floor(log10(ROW_INDEX)) + 1;
    const char* TXT_FILE_PATH = argv[3];
    const int CHARS_TO_READ = atoi(argv[4]);

    FILE* source=fopen(TXT_FILE_PATH, "r");
    if(source== NULL){
        printf("Nie udało się otworzyć pliku tekstowego %s.", TXT_FILE_PATH);
        return EXIT_FAILURE;
    }

    FILE* pipe = fopen(PIPE_PATH, "w");
    if(pipe == NULL) {
        printf("Nie udało się dostać do potoku %s.", PIPE_PATH);
        return EXIT_FAILURE;
    }

    char* chars_to_write_out = malloc(CHARS_TO_READ * sizeof(char));
    char* all_to_write_out = malloc((CHARS_TO_READ+ROW_INDEX_SIZE+3) * sizeof(char));

    while(fread(chars_to_write_out, sizeof(char), CHARS_TO_READ, source) == CHARS_TO_READ){
        sprintf(all_to_write_out, "$%d$%s", ROW_INDEX, chars_to_write_out);
        fwrite(all_to_write_out, sizeof(char),CHARS_TO_READ+ROW_INDEX_SIZE+2, pipe);
        sleep(1);
    }

    free(chars_to_write_out);
    free(all_to_write_out);
    fclose(pipe);
    fclose(source);
}
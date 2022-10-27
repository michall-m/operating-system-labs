#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "pgm_lib.h"


typedef enum division_t {
    NUMBERS=0,
    BLOCK=1
}   division_t;


typedef struct timespec tspec;
long unsigned int time_it(tspec start, tspec stop) {
    return (stop.tv_sec-start.tv_sec)*1000000000 + (stop.tv_nsec-start.tv_nsec);
};

pgm_file_s* pgm_input;
pgm_file_s* negative_pgm;

int no_threads;

void* numbers_method(void* arg);
void* block_method(void* arg);

int main(int argc, char** argv) {
    if (argc-1 != 4) {
        printf("Program przyjmuje następujące argumenty:\n"
               "\n"
               "liczbę wątków,\n"
               "sposób podziału obrazu pomiędzy wątki, t.j. jedną z dwóch opcji: numbers / block\n"
               "nazwę pliku z wejściowym obrazem,\n"
               "nazwę pliku wynikowego.");
        //exit(EXIT_FAILURE);
    }

    //arguments parsing
    division_t division;
    char *input_filename, *output_filename;

    no_threads = atoi(argv[1]);

    if (strcmp(argv[2], "numbers") == 0) {
        division = NUMBERS;
    } else if (strcmp(argv[2], "block") == 0) {
        division = BLOCK;
    } else {
        printf("Błędny sposób podziału.");
        exit(EXIT_FAILURE);
    }
    char* method_string = argv[2];
    input_filename = argv[3];
    output_filename = argv[4];

    pgm_input = parse_pgm_file(input_filename);

    //allocate struct for output
    negative_pgm = malloc(sizeof(pgm_file_s));
    negative_pgm->gray_values = calloc(pgm_input->height, sizeof(int*));
    for (int i = 0; i < pgm_input->height; i++) {
        negative_pgm->gray_values[i] = calloc(pgm_input->width, sizeof(int));
    }
    negative_pgm->max_gray_value = pgm_input->max_gray_value;
    negative_pgm->height = pgm_input->height;
    negative_pgm->width = pgm_input->width;


    pthread_t* pthreads = calloc(no_threads, sizeof(pthread_t));
    int* arguments = calloc(no_threads, sizeof(int));

    tspec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);

    switch (division) {
        case NUMBERS: {
            for (int i = 0; i < no_threads; ++i) {
                arguments[i] = i;
                pthread_create(&pthreads[i], NULL, &numbers_method, &arguments[i]);
            }
        }
            break;
        case BLOCK: {
            for (int i = 0; i < no_threads; ++i) {
                arguments[i] = i;
                pthread_create(&pthreads[i], NULL, &block_method, &arguments[i]);
            }
        }
            break;
        default:
            printf("Division enum: bad arg.");
            exit(EXIT_FAILURE);
    }

    printf("\n-----------------------------------------\n");
    printf("Number of threads: %d.\n", no_threads);
    printf("Used method: %s.\n", method_string);
    printf("=========================================\n");



    for(int i = 0; i < no_threads; i++) {
        long unsigned int* timed;
        pthread_join(pthreads[i], (void **) &timed);

        printf("Thread index: %i.\t Time: %ld[us].\n", i, *timed/1000);
        free(timed);
    }
    printf("-   -   -   -   -   -   -   -   -   -   -\n");

    clock_gettime(CLOCK_REALTIME, &stop);
    long unsigned int timed_total =  time_it(start, stop);
    printf("Method total time: %ld[us].\n", timed_total/1000);
    printf("-----------------------------------------\n");


    write_pgm_to_file(output_filename, negative_pgm);
    free_pgm_file_s(pgm_input);
    free_pgm_file_s(negative_pgm);
    free(pthreads);
    free(arguments);
    
    return EXIT_SUCCESS;
}


void* block_method(void* arg) {
    tspec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    int index = *(int*) arg;
    int multiplier = ceil((double) pgm_input->width/no_threads);
    int p,q;
    p = index*multiplier;
    q = (index == no_threads-1) ? pgm_input->width : (index+1)*multiplier;
    for(int i = 0; i < pgm_input->height; i++) {
        for (int j = p; j < q; j++) {
            negative_pgm->gray_values[i][j] = pgm_input->max_gray_value - pgm_input->gray_values[i][j];
        }
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    long unsigned int* timed = malloc(sizeof(long unsigned int));
    *timed = time_it(start, stop);
    pthread_exit(timed);
}

void* numbers_method(void* arg) {
    tspec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    int index = *(int*) arg;
    int multiplier = ceil((double) pgm_input->max_gray_value/no_threads);

    int p,q;
    p = index*multiplier;
    q = (index == no_threads-1) ? pgm_input->max_gray_value+1 : (index+1)*multiplier;
    for(int i = 0; i < pgm_input->height; i++) {
        for (int j = 0; j < pgm_input->width; j++) {
            if(pgm_input->gray_values[i][j] >= p && pgm_input->gray_values[i][j] < q)
            negative_pgm->gray_values[i][j] = pgm_input->max_gray_value - pgm_input->gray_values[i][j];
        }
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    long unsigned int* timed = malloc(sizeof(long unsigned int));
    *timed = time_it(start, stop);
    pthread_exit(timed);
}
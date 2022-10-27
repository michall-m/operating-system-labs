#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

void exec_files_merging(char *cmd);
void measure_time(struct timespec *rl_start, struct tms *t_start);


int main(int argc, char** argv){

    int no_cmds;
    scanf("%d", &no_cmds);
    getchar();

    while (no_cmds--){
        char *cmd = NULL;
        size_t b_size = 0;
        getline(&cmd, &b_size, stdin);

        exec_files_merging(cmd);

        free(cmd);
    }
    delete_temporary_files();
}

void exec_files_merging(char *cmd){
    char* test_name = strtok(cmd, " ");
    printf("TEST: %s\n", test_name);

    char* number = strtok(NULL, " ");
    int n = atoi(number);

    char* seq = strtok(NULL, "\n");

    struct timespec *rl_start = malloc(sizeof(struct timespec));
    struct tms *t_start = malloc(sizeof(struct tms));

    clock_gettime(CLOCK_REALTIME, rl_start);
    times(t_start);


    //file_pairs_sequence* fps = malloc(sizeof(*fps));
    file_pairs_sequence* fps = parse_sequence(seq);
    merge_to_temporary_files(fps);

    for (int i = 0; i < n; i++){
        fflush(stdout);
        if (fork() == 0){
            merge_to_temporary_files(fps);
            exit(0);
        }
    }

    //printf("%s %s\n", test_name, merge_cmd);
    measure_time(rl_start, t_start);
    free(fps);
    free(rl_start);
    free(t_start);
    while(n--){
        wait(NULL);
    }
    //return fps;
}

void measure_time(struct timespec *rl_start, struct tms *t_start){
    struct timespec *rl_stop = malloc(sizeof(struct timespec));
    struct tms *t_stop = malloc(sizeof(struct tms));
    const double ns = 1000000000;

    clock_gettime(CLOCK_REALTIME, rl_stop);
    times(t_stop);

    printf("czas rzeczywisty:\t%.8f\nczas użytkownika:\t%.8f\nczas systemowy:  \t%.8f\n\n\n",
           (double) (rl_stop->tv_sec - rl_start->tv_sec) +
           (double) (rl_stop->tv_nsec - rl_start->tv_nsec)/ns,
           (double) (t_stop->tms_cutime - t_start->tms_cutime) / CLOCKS_PER_SEC,
           (double) (t_stop->tms_cstime - t_start->tms_cstime) / CLOCKS_PER_SEC);
    free(rl_stop);
    free(t_stop);
}
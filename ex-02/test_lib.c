#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library.h>
#include <time.h>
#include <sys/times.h>

//mergowanie wszystkich
//cmd T_nazwa_testu para:tara tara:para
file_pairs_sequence* exec_files_merging(char* cmd);

//cmd T_nazwa_testu
main_array* exec_array_creating(char* cmd, file_pairs_sequence *fps);

//usuwanie wszystkiego
//cmd T_nazwa_testu
void exec_blocks_removing(char* cmd, main_array *ma);

//dodawanie wszystkiego
//cmd T_nazwa_testu ????ilosc_razy
void exec_blocks_add_remove_operation(char *cmd, file_pairs_sequence *fps, main_array *ma);


void measure_time(struct timespec *rl_start, struct tms *t_start);


const char* merge_cmd = "test_merge_files";
const char* create_array_cmd = "test_create_array";
const char* remove_blocks_cmd = "test_remove_blocks";
const char* add_remove_blocks_cmd = "test_add_remove_blocks";
/*
test_merge_files testname text1.txt:text2.txt
test_create_array testname
test_remove_blocks testname
test_add_remove_blocks testname
 */
int main(){
    int no_cmds;
    scanf("%d", &no_cmds);
    getchar();
    system("pwd");
    //data
    main_array *ma;
    file_pairs_sequence *fps;

    while (no_cmds--){
        char *cmd = NULL;
        size_t b_size = 0;
        getline(&cmd, &b_size, stdin);

        if(strncmp(merge_cmd, cmd, strlen(merge_cmd)) == 0){
            fps = exec_files_merging(cmd);
        }
        else if (strncmp(create_array_cmd, cmd, strlen(create_array_cmd)) == 0){
            ma = exec_array_creating(cmd, fps);
        }
        else if (strncmp(remove_blocks_cmd, cmd, strlen(remove_blocks_cmd)) == 0){
            exec_blocks_removing(cmd, ma);
        }
        else if (strncmp(add_remove_blocks_cmd, cmd, strlen(add_remove_blocks_cmd)) == 0){
            exec_blocks_add_remove_operation(cmd, fps, ma);
        }
        free(cmd);
    }
    print_merged_files(ma);
}

file_pairs_sequence* exec_files_merging(char *cmd){
    char *test_name = strtok(cmd+strlen(merge_cmd), " ");
    char *seq = strtok(NULL, "\n");

    struct timespec *rl_start = malloc(sizeof(struct timespec));
    struct tms *t_start = malloc(sizeof(struct tms));

    clock_gettime(CLOCK_REALTIME, rl_start);
    times(t_start);

    file_pairs_sequence* fps = malloc(sizeof(*fps));
    fps = parse_sequence(seq);
    merge_to_temporary_files(fps);

    //printf rodzaj testu, nazwa testu
    measure_time(rl_start, t_start);

    free(rl_start);
    free(t_start);

    return fps;
}

main_array* exec_array_creating(char* cmd, file_pairs_sequence* fps){
    char *test_name = strtok(cmd + strlen(create_array_cmd) + 1, "\n");

    struct timespec *rl_start = malloc(sizeof(struct timespec));
    struct tms *t_start = malloc(sizeof(struct tms));

    clock_gettime(CLOCK_REALTIME, rl_start);
    times(t_start);

    main_array* ma = create_main_array();
    fill_main_array(ma, fps);

    //printf rodzaj testu, nazwa testu
    measure_time(rl_start, t_start);

    free(rl_start);
    free(t_start);

    return ma;
}

void exec_blocks_removing(char* cmd, main_array *ma){
    char *test_name = strtok(cmd + strlen(remove_blocks_cmd) + 1, "\n");

    struct timespec *rl_start = malloc(sizeof(struct timespec));
    struct tms *t_start = malloc(sizeof(struct tms));

    clock_gettime(CLOCK_REALTIME, rl_start);
    times(t_start);

    for(int i = 0; i < ma->length; i++){
        remove_block(ma, i);
    }
    ma->length = 0;

    //printf rodzaj testu, nazwa testu
    measure_time(rl_start, t_start);

    free(rl_start);
    free(t_start);
}

void exec_blocks_add_remove_operation(char *cmd, file_pairs_sequence *fps, main_array *ma){
    char *test_name = strpbrk(cmd + strlen(add_remove_blocks_cmd) + 1, "\n") + 1;

    struct timespec *rl_start = malloc(sizeof(struct timespec));
    struct tms *t_start = malloc(sizeof(struct tms));

    clock_gettime(CLOCK_REALTIME, rl_start);
    times(t_start);


    fill_main_array(ma, fps);
    for(int i = 0; i < ma->length; i++){
        remove_block(ma, i);
    }
    ma->length = 0;

    //printf rodzaj testu, nazwa testu
    measure_time(rl_start, t_start);

    free(rl_start);
    free(t_start);
}

void measure_time(struct timespec *rl_start, struct tms *t_start){
    struct timespec *rl_stop = malloc(sizeof(struct timespec));
    struct tms *t_stop = malloc(sizeof(struct tms));
    const int ns = 1000000000;

    clock_gettime(CLOCK_REALTIME, rl_stop);
    times(t_stop);

    printf("czas rzeczywisty: %.6f\t czas użytkownika: \t%.6f czas systemowy: \t%.6f\n",
           (double) ((rl_stop->tv_sec - rl_start->tv_sec) +
                     (double)((rl_stop->tv_nsec - rl_stop->tv_nsec)/ns)),
           (double) (t_stop->tms_cutime - t_start->tms_cutime) / CLOCKS_PER_SEC,
           (double) (t_stop->tms_cstime - t_start->tms_cstime / CLOCKS_PER_SEC));

    free(rl_stop);
    free(t_stop);
}
#include "lib.h"

main_array* create_main_array(){
    main_array* ma = malloc(sizeof(main_array));
    ma -> blocks = NULL;
    ma -> length = 0;
    ma -> size = 0;
    return ma;
}

file_pairs_sequence* parse_sequence(char* input){
    unsigned int no_pairs = 0;
    for(int i = 0; i < strlen(input); i++){
        if (input[i] == ':'){
            no_pairs++;
        }
    }
    file_pairs_sequence *sequence = malloc(sizeof(file_pairs_sequence));
    sequence->length = no_pairs;

    sequence->pairs = calloc(no_pairs, sizeof(file_pair*));
    sequence->pairs[0] = malloc(sizeof(file_pair));
    sequence->pairs[0] -> first_filename = strtok(input, " :");
    sequence->pairs[0] -> second_filename = strtok(NULL, " :");
    for(int i = 1; i < no_pairs; i++){
        sequence->pairs[i] = malloc(sizeof(file_pair));
        sequence->pairs[i] -> first_filename = strtok(NULL, " :");
        sequence->pairs[i] -> second_filename = strtok(NULL, " :");
    }
    return sequence;
}

void merge_to_temporary_files(file_pairs_sequence* sequence){
    for(int i = 0; i < sequence->length; i++){
        unsigned int cmd_len = 20 + strlen(sequence->pairs[i]->first_filename) + strlen(sequence->pairs[i]->second_filename);
        char* cmd = calloc(cmd_len, sizeof(char));
        sequence->pairs[i]->pair_id = calloc(10 + (i+1)/10, sizeof(char));            // ?
        sprintf(sequence->pairs[i]->pair_id, "tmp:%d.txt", (i+1));                        //  ?
        sprintf(cmd, "paste -d \"\\n\" %s %s > %s",
                sequence->pairs[i]->first_filename,
                sequence->pairs[i]->second_filename,
                sequence->pairs[i]->pair_id);
        system(cmd);
        free(cmd);
    }
}

void delete_temporary_files(){
    system("rm tmp:*");
    /*
    for(int i = 0; i < sequence->length; i++){
        char* cmd = calloc(4 + strlen(sequence->pairs[i]->pair_id), sizeof(char));
        sprintf(cmd, "rm %s", sequence->pairs[i]->pair_id);
        system(cmd);
    }*/
}

int create_rows_block(main_array *ma, char* filename){
    rows_block* new_rb = malloc(sizeof(rows_block*));
    new_rb->rows = NULL;
    new_rb->length = 0;
    new_rb->size = 0;


    FILE *file = fopen(filename, "r");
    if (file == NULL){
        return -1;
    }

    char* row = NULL;
    size_t r_size = 0;

    while(getline(&row, &r_size, file) != -1) {
        if (new_rb->length == new_rb->size) {
            if (new_rb->size > 0) {
                new_rb->size *= 2;
                new_rb->rows = realloc(new_rb->rows, new_rb->size * sizeof(char *));
            } else {
                new_rb->size = 1;
                new_rb->rows = malloc(sizeof(char *));
            }
        }
        new_rb->rows[new_rb->length] = row;
        new_rb->length++;
        row = NULL;
        r_size = 0;
    }
    fclose(file);
    free(row);

    if (ma ->length == ma ->size){
        if(ma-> size > 0){
            ma ->size *=2;
            ma ->blocks = realloc(ma->blocks, ma->size * sizeof(rows_block*));
        } else {
            ma ->size = 1;
            ma ->blocks = malloc(sizeof(rows_block*));
        }
    }
    ma->blocks[ma->length] = new_rb;
    ma->length++;
    return ma->length - 1;
}

void fill_main_array(main_array *ma, file_pairs_sequence *sequence){
    for(int i = 0; i < sequence->length; i++){
        create_rows_block(ma, sequence->pairs[i]->pair_id);
    }
}

int count_rows(rows_block *rb){
    unsigned int r=0;
    for (int i = 0; i < rb->length; i++){
        if (rb->rows[i] == NULL){
            continue;
        }
        r++;
    }
    return r;
}

void remove_row(rows_block *rb, int row_index){
    if (rb->length < row_index){
        return;
    }
    if (rb->rows[row_index] == NULL){
        return;
    }
    free(rb->rows[row_index]);
    rb->rows[row_index] = NULL;
}

void remove_block(main_array *ma, int block_index){
    if (block_index > ma->length){
        return;
    }
    if (ma->blocks[block_index] == NULL){
        return;
    }
    for (int i = 0; i < ma->blocks[block_index]->length; i++){
        remove_row(ma->blocks[block_index], i);
    }
    free(ma->blocks[block_index]);
    ma->blocks[block_index] = NULL;
}

void remove_sequence(file_pairs_sequence *fps){
    for(int i = 0; i < fps->length; i++){
        free(fps->pairs[i]->first_filename);
        free(fps->pairs[i]->second_filename);
        free(fps->pairs[i]->pair_id);
    }
    free(fps->pairs);
    free(fps);
}

void print_merged_files(main_array *ma){
    for(int i = 0; i < ma->length; i++){
        if(ma->blocks[i] == NULL){
            continue;
        }
        printf("| %d. block |\n", i);
        for(int j = 0; j < ma->blocks[i]->length; j++){
            if (ma->blocks[i]->rows[j] == NULL){
                continue;
            }
            printf("%s", ma->blocks[i]->rows[j]);
        }
    }
}
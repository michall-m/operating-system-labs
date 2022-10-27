#ifndef lib
#define lib

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct rows_block {
    unsigned int length;
    unsigned int size;
    char** rows;
} rows_block;

typedef struct main_array {
    unsigned int length;
    unsigned int size;
    rows_block** blocks;
} main_array;


typedef struct file_pair {
    char* first_filename;
    char* second_filename;
    char* pair_id;
} file_pair;

typedef struct file_pairs_sequence{
    int length;
    file_pair** pairs;
} file_pairs_sequence;

main_array* create_main_array();
file_pairs_sequence* parse_sequence(char* input);
void merge_to_temporary_files(file_pairs_sequence* sequence);
void delete_temporary_files();
int create_rows_block(main_array* ma, char* merged_filename);
void fill_main_array(main_array *ma, file_pairs_sequence *sequence);
int count_rows(rows_block *block);
void remove_block(main_array *ma, int block_index);
void remove_row(rows_block *rb, int row_index);
void print_merged_files(main_array *ma);
void remove_sequence(file_pairs_sequence *fps);
#endif
#include "pgm_lib.h"

void free_pgm_file_s(pgm_file_s* pgm_file) {
    for (int i = 0; i < pgm_file->height; i++) {
        free(pgm_file->gray_values[i]);
    }
    free(pgm_file->gray_values);
    free(pgm_file);
}

pgm_file_s* parse_pgm_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }
    pgm_file_s* pgm_file = malloc(sizeof(pgm_file_s));
    //parse first line
    do {
        char* line = NULL;
        size_t buf = 0;
        if (getline(&line, &buf, file) == -1){
            perror("Parsing error: no first line.");
            exit(EXIT_FAILURE);
        }

        if (line[0] == '#') {
            free(line);
            continue;
        }

        free(line);
        break; // we don't need to know the first line
    } while(1);

    //parse second line -> width, height
    do {
        char* line = NULL;
        size_t buf = 0;
        if (getline(&line, &buf, file) == -1) {
            perror("Parsing error: no second line");
            exit(EXIT_FAILURE);
        }
        if (line[0] == '#') {
            free(line);
            continue;
        }

        strtok(line, " ");
        pgm_file->width = atoi(line);

        char* after_sep_line = line + strlen(line) + 1;
        strtok(after_sep_line, "\n#");
        pgm_file->height = atoi(after_sep_line);
        free(line);
        break;
    } while(1);

    //now we can allocate memory
    pgm_file->gray_values = calloc(pgm_file->height, sizeof(int*));
    for (int i = 0; i < pgm_file->height; i++) {
        pgm_file->gray_values[i] = calloc(pgm_file->width, sizeof(int));
    }

    //parse third line -> max value of gray
    do {
        char* line = NULL;
        size_t buf = 0;
        if (getline(&line, &buf, file) == -1) {
            perror("Parsing error: no second line");
            exit(EXIT_FAILURE);
        }
        if (line[0] == '#') {
            free(line);
            continue;
        }
        strtok(line, "\n");
        pgm_file->max_gray_value = atoi(line);
        free(line);
        break;
    } while(1);

    int line_no = 5;
    //parse pixel values
    int index = 0;
    do {
        char* line = NULL;
        size_t buf = 0;
        if (getline(&line, &buf, file) == -1) {
            break;
        }
        char* value;
        value = strtok(line, " \n");
        while(value != NULL) {
            int row = index/pgm_file->width;
            int column = index%pgm_file->width;
            pgm_file->gray_values[row][column] = atoi(value);
            index++;
            value = strtok(NULL, " \n");
        }
        free(line);
        index--;
        line_no++;
    } while (1);

    fclose(file);
    return pgm_file;
}

int write_pgm_to_file(char* filename, pgm_file_s* pgm_file) {
    FILE* file = fopen(filename, "w");

    //convert first 3 lines to ascii
    char magic_number[MAX_VAL_LEN] = "P2\n";
    char width_height[MAX_VAL_LEN];
    sprintf(width_height, "%d %d\n", pgm_file->width, pgm_file->height);
    char ascii_max_gray_value[MAX_VAL_LEN];
    sprintf(ascii_max_gray_value, "%d\n", pgm_file->max_gray_value);

    fwrite(magic_number, sizeof(char), strlen(magic_number), file);
    fwrite(width_height, sizeof(char), strlen(width_height), file);
    fwrite(ascii_max_gray_value, sizeof(char), strlen(ascii_max_gray_value), file);

    //write out all the values, row length < 70 as mentioned in header
    int row_len_counter = 1;
    for (int row = 0; row < pgm_file->height; row++) {
        for (int column = 0; column < pgm_file->width; column++) {
            char gray_value[MAX_VAL_LEN];
            sprintf(gray_value, "%d ", pgm_file->gray_values[row][column]);
            fwrite(gray_value, sizeof(char), strlen(gray_value), file);
            if (row_len_counter++ >= 70 && !(column == pgm_file->width && row == pgm_file->height)) {
                fwrite("\n", sizeof(char), strlen("\n"), file);
                row_len_counter = 1;
            }
        }
    }

    fclose(file);
    return 1;
}
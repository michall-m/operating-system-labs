//
// Created by Michał Misiak on 14.05.2021.
//

#ifndef POSIX_PGM_LIB_H
#define POSIX_PGM_LIB_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_VAL_LEN 16

typedef struct pgm_file_s {
    int width;
    int height;
    int max_gray_value;
    int** gray_values;
} pgm_file_s;


/*
 * Format:
 * > A "magic number" for identifying the file type.
 *   An ASCII PGM file's magic number is the two characters "P2".
 * > A width, formatted as ASCII characters in decimal.
 * > A height, again in ASCII decimal.
 * > The maximum gray value, again in ASCII decimal.
 * > Width * height gray values, each in ASCII decimal,
 *   between 0 and the specified maximum value, separated by whitespace,
 *   starting at the top-left corner of the graymap,
 *   proceeding in normal English reading order.
 *   A value of 0 means black, and the maximum value means white.
 *
 * Additional information:
 * > Characters from a "#" to the next end-of-line are ignored (comments).
 * > No line should be longer than 70 characters.
 *
 *
 *
 *
 *
 */
pgm_file_s* parse_pgm_file(char* filename);
void free_pgm_file_s(pgm_file_s* pgm_file);
int write_pgm_to_file(char* filename, pgm_file_s* pgm_file);

#endif //POSIX_PGM_LIB_H

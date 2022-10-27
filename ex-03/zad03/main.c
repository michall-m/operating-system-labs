#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

const char* FIRST_DIRECTORY_PATH;
const char* PATTERN;
int MAX_DEPTH;

static int FORKS = 0;

const char* EXTENSION = ".txt";
const int EXTENSION_NAME_LEN = 4;
const int MAX_PATH_LEN = 256;

int is_extension_proper(char* filename);
int does_contain_pattern(char* path, char* filename);
void print_result(char* current_path, char* filename);
void search_for_files(char* path, int depth);
void clear_processes();

int main(int argc, char** argv){
    if (argc - 1 < 3){
        printf("Not enough args.");
        return -1;
    }
    if (argc - 1 > 3){
        printf("Too many args.");
        return -1;
    }

    FIRST_DIRECTORY_PATH = argv[1];
    PATTERN = argv[2];
    MAX_DEPTH = atoi(argv[3]);

    printf("searching...\n");
    search_for_files(FIRST_DIRECTORY_PATH, 0);
    clear_processes();
}


int is_extension_proper(char* filename){
    if(strlen(filename) <= EXTENSION_NAME_LEN){
        return 0;
    }
    if (strcmp(filename + strlen(filename) - EXTENSION_NAME_LEN, EXTENSION)){
        return 0;
    }
    return 1;
}

int is_directory_proper(char* directoryname){
    if(strcmp(directoryname, ".") == 0){
        return 0;
    }
    if(strcmp(directoryname, "..") == 0){
        return 0;
    }
    return 1;
}

int does_contain_pattern(char* path, char* filename){
    char filepath[MAX_PATH_LEN];
    strcpy(filepath, path);
    strcat(filepath, "/");
    strcat(filepath, filename);

    FILE* file = fopen(filepath, "r");

    char letter;
    int eof = 1;
    while(eof){
        int counter = 0;
        while(eof =  fread(&letter, sizeof(char), 1, file) == 1){
            if(letter == PATTERN[counter]){
                counter++;
                if(counter == strlen(PATTERN)){
                    return 1;
                }
            } else {
                counter = 0;
            }

            if(letter == '\n'){
                break;
            }
        }
    }
    fclose(file);
    return 0;
}

void print_result(char* path, char* filename){
    char filepath[MAX_PATH_LEN];
    strcpy(filepath, path);
    strcat(filepath, "/");
    strcat(filepath, filename);
    printf("PID: %d\t Relative path: .%s\n", getpid(), filepath + strlen(FIRST_DIRECTORY_PATH));
}

void clear_processes(){
    printf("waiting...\n");
    while(FORKS--){
        wait(NULL);
    }
    printf("done\n");
}


void search_for_files(char* path, int depth){
    if (depth > MAX_DEPTH){
        exit(0);
    }

    DIR* directory;
    if(NULL == (directory = opendir(path))){
        exit(0);
    }

    struct dirent* dirent;

    while (NULL != (dirent = readdir(directory))){
        if(dirent->d_type == DT_REG && is_extension_proper(dirent->d_name) && does_contain_pattern(path, dirent->d_name)){
            print_result(path, dirent->d_name);
            continue;
        }

        if(dirent->d_type == DT_DIR && is_directory_proper(dirent->d_name)){
            if(fork() == 0){
                FORKS++;
                char filepath[MAX_PATH_LEN];
                strcpy(filepath, path);
                strcat(filepath, "/");
                strcat(filepath, dirent->d_name);
                closedir(directory);
                search_for_files(filepath, depth+1);
                exit(0);
            }
        }
    }
    closedir(directory);
}
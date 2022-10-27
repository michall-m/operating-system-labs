#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#define MAX_ELEMENTS 10
#define MAX_COMMANDS 10
#define MAX_ARGS 10
#define MAX_ELEMENTS_TO_EXEC 64
#define MAX_PIPE_ELEMENTS 64

typedef struct Command {
    char* cmd;
    char** argv;
    int argc;
} Command;

typedef struct Element {
    int k;
    Command** cmdv;
    int cmdc;
} Element;

Command* parse_command(char* arg){
    Command* cmd = malloc(sizeof(Command));
    char* cmd_name = strtok(arg, " ");
    cmd->cmd = malloc(sizeof(cmd_name));
    strcpy(cmd->cmd, cmd_name);
    cmd->argv = malloc(MAX_ARGS*sizeof(char*));
    int i = 0;
    char* cmd_argument;
    while((cmd_argument = strtok(NULL, " ")) != NULL){
        cmd->argv[i] = malloc(sizeof(cmd_argument));
        strcpy(cmd->argv[i], cmd_argument);
        i++;
    }
    cmd->argc = i;
    cmd->argv[i] = NULL;
    return cmd;
}

Element* parse_line(char* arg){
    char* arguments = strchr(arg, '=') + 2;
    strtok(arg, "\n");
    char* command_string = strtok(arguments, "|");
    Element* e = malloc(sizeof(Element));
    e->cmdv = malloc(MAX_COMMANDS * sizeof(Command*));
    e->cmdc = 0;
    char * next_command_string =command_string+strlen(command_string)+1;
    while(command_string != NULL){
        e->cmdv[e->cmdc] = malloc(sizeof(Command));
        e->cmdv[e->cmdc] = parse_command(command_string);
        e->cmdc++;
        command_string = strtok(next_command_string, "|");
        if(strcmp(next_command_string, "\n") == 0){
            break;
        }
        if(command_string != NULL) {
            next_command_string = command_string + strlen(command_string) + 1;
        }
    }
    return e; //sprawdzac zawsze cmdc
}

void execute_pipe(Element** elements, int* to_exec, int execs, int no_pipe){
    /*
     * Counting commands.
     */
    int commands_counter = 0;
    for(int i = 0; i < execs; i++){
        int k = to_exec[i];
        commands_counter += elements[k-1]->cmdc;
    }
    const int commands_to_execute = commands_counter;
    int fd[commands_to_execute][2];
    for(int i = 0; i < commands_to_execute; i++){
        pipe(fd[i]);
    }

    /*
     * Execute
     */
    int fd_id = 0;
    for(int i = 0; i < execs; i++){
        int k = to_exec[i];
        for(int j = 0; j < elements[k-1]->cmdc; j++){
            char* arguments[MAX_ARGS + 2];
            arguments[0] = elements[k-1]->cmdv[j]->cmd;
            for (int q = 0; q <= elements[k-1]->cmdv[j]->argc; q++){
                arguments[q+1] = elements[k-1]->cmdv[j]->argv[q];
            }
            if (fork() == 0){
                if(fd_id != 0){
                    dup2(fd[fd_id-1][0], STDIN_FILENO);
                }
                if(fd_id != commands_to_execute-1){
                    dup2(fd[fd_id][1], STDOUT_FILENO);
                }
                for(int x = 0; x < commands_to_execute; x++){
                    close(fd[x][1]); // ?????
                }
//                if(fd_id != 0) {
//                    close(fd[fd_id - 1][1]);
//                }
                execvp(elements[k-1]->cmdv[j]->cmd, arguments);
            } else {
                fd_id++;
            }
        }
    }
    for(int i = 0; i < commands_to_execute; i++){
        close(fd[i][1]);
    }

    for(int i = 0; i < commands_to_execute; i++){
        wait(NULL);
    }

}

void free_mem(Element** elements, int counter){
    for(int i = 0; i < counter; i++){
        for(int j = 0; j < elements[i]->cmdc; j++){
            free(elements[i]->cmdv[j]->cmd);
            for(int p = 0; p < elements[i]->cmdv[j]->argc; p++){
                free(elements[i]->cmdv[j]->argv[p]);
            }
            free(elements[i]->cmdv[j]->argv);
            free(elements[i]->cmdv[j]);
        }
        free(elements[i]->cmdv);
        free(elements[i]);
    }
    free(elements);
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Pass one argument - name of the file.");
        return EXIT_FAILURE;
    }
    if(freopen(argv[1], "r", stdin) == NULL){
        printf("Unable to open the file.");
        return EXIT_FAILURE;
    }

    if(freopen("OUTPUTCW04ZAD01.txt", "w", stdout) == NULL){
        printf("output problem.");
        return EXIT_FAILURE;
    }

    /*
     * PARSING
     */
    Element** elements = malloc(MAX_ELEMENTS * sizeof(Element*));

    char* element_to_parse;
    size_t b = 0;
    int k = 0; // k -> k+1 w poleceniu

    while(getline(&element_to_parse, &b, stdin) > 1){
        Element* el = parse_line(element_to_parse);
        elements[k] = malloc(sizeof(Element));
        elements[k++] = el;
        b = 0;
    }

    char* exec_to_parse;
    int elements_to_exec[MAX_PIPE_ELEMENTS][MAX_ELEMENTS_TO_EXEC];
    int execs[MAX_PIPE_ELEMENTS];
    for(int i = 0; i < MAX_PIPE_ELEMENTS; i++){
        execs[i] = 0;
    }
    int elements_pipes = 0;

    while(getline(&exec_to_parse, &b, stdin) > 1){
        char* last_toatoi = strtok(exec_to_parse, " ") + strlen("składnik");
        elements_to_exec[elements_pipes][execs[elements_pipes]++] = atoi(last_toatoi);

        char* toatoi;
        int dd = floor(log10(elements_to_exec[elements_pipes][execs[elements_pipes]-1])) + 1;
        if(strcmp(last_toatoi + dd, "\n") == 0){
            elements_pipes++;
            continue;
        }
        last_toatoi += dd + strlen(" | ");
        while ((toatoi= strtok(last_toatoi, " ") + strlen("skladnikx")) != NULL){
            elements_to_exec[elements_pipes][execs[elements_pipes]++] = atoi(toatoi);
            dd = floor(log10(elements_to_exec[elements_pipes][execs[elements_pipes]-1])) + 1;
            last_toatoi = toatoi + dd;
            if(strcmp(last_toatoi, "\n") == 0){
                break;
            }
            last_toatoi += strlen(" | ");
        }
        b = 0;
        elements_pipes++;
    }

    for(int i = 0; i < elements_pipes; i++){
        execute_pipe(elements, elements_to_exec[i], execs[i], i);
    }
    free_mem(elements, k);
    fclose(stdout);
    fclose(stdin);
}
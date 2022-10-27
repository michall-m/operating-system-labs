#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "server_config.h"

int client_qid;
int client_id;
int server_qid;

int connected_to_qid = -1;
int connected_to_id = -1;//if -1 not connected

void rqt_stop(){ // do zmiany
    msg_buffer msgbuf = {.mtype = REQTYPE_STOP, .sender_qid = client_qid,};
    if (msgsnd(server_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
        perror("stop failed");
    }
    sleep(1);
    msgctl(client_qid, IPC_RMID, NULL);
    exit(0);
}

void init_client(server_qid){
    msg_buffer msgbuf_init = {.mtype = REQTYPE_INIT, .sender_qid = client_qid};
    if (msgsnd(server_qid, &msgbuf_init, sizeof(msgbuf_init), NO_FLAG) == -1){
        perror("initialization failed");
        exit(-1);
    }
    //TODO stop request
};

int client_command(char* cmd){
    if(strcmp(cmd, "!STOP\n") == 0){
        return REQTYPE_STOP;
    }
    if(strcmp(cmd, "!DISCONNECT\n") == 0){
        return REQTYPE_DISCONNECT;
    }
    if(strcmp(cmd, "!LIST\n") == 0){
        return REQTYPE_LIST;
    }
    if(strncmp(cmd, "!CONNECT ", strlen("!CONNECT ")) == 0){ //TODO IDK CZY ! WIADOMOSC TO GIT
        return REQTYPE_CONNECT;
    }
    if(strcmp(cmd, "!LIST\n") == 0){
        return REQTYPE_LIST;
    }
    return REQTYPE_MSG;
}

/*
 * REQUESTS
 */
int msg_request();
int stop_request();
int disconnect_request();
int connect_request(int client_ID, int server_qid);
int list_request(int server_qid);

int catch_reply(long type, int server_qid);


int main(){
    /*
     * KEYS
     */
    key_t server_key, client_key;
    server_key = ftok($HOME, PROJ_CHAR_ID);
    client_key = ftok($HOME, getpid());
    if (server_key == -1){
        perror("server key error");
    }
    if (client_key == -1){
        perror("client key error");
    }

    /*
     * QUEUE IDs
     */
    server_qid = msgget(server_key, 0);
    client_qid = msgget(client_key, IPC_CREAT | 0666);
    if (server_qid == -1){
        perror("server qid error");
    }
    if(client_qid == -1){
        perror("client qid error");
    }

    /*
     * CTRL + C HANDLER
     */
    signal(SIGINT, rqt_stop);


    /*
     * INIT
     */
    init_client(server_qid);



    /*
     * MESSAGES HANDLER
     */

    while(1){
        struct timeval timeout;
        fd_set readfds;
        timeout.tv_sec = 0;
        timeout.tv_usec = 50;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        if((FD_ISSET(0, &readfds))) {
            char *line = NULL;
            size_t buf;
            getline(&line, &buf, stdin);
            switch (client_command(line)) {
                case REQTYPE_MSG: {
                    if (connected_to_id == -1){
                        printf("First you have to !CONNECT someone.\n");
                        break;
                    }
                    msg_buffer msgbuf = {.mtype = REQTYPE_MSG, .sender_id = client_id};
                    strcat(msgbuf.mtext, line);
                    printf("YOU | %s", line);
                    if (msgsnd(connected_to_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
                        perror("msg sending failed");
                    }
                }
                    break;
                case REQTYPE_STOP:
                    rqt_stop();
                    break;
                case REQTYPE_DISCONNECT: {
                    msg_buffer msgbuf = {.mtype = REQTYPE_DISCONNECT, .sender_qid = client_qid};
                    if (msgsnd(server_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1) {
                        perror("disconnect request failed");
                    }
                    connected_to_id = -1;
                    connected_to_qid = -1;
                    printf("#Disconected succesfully.\n");
                }
                    break;
                case REQTYPE_CONNECT: {
                    int client_ID = atoi(line + strlen("!COMMAND "));
                    connect_request(client_ID, server_qid);
                    msg_buffer msgbuf;
                    if (msgrcv(client_qid, &msgbuf, sizeof(msgbuf), REQTYPE_NEGATIVE_LAST, NO_FLAG) == -1) {
                        perror("catch reply error");
                        return 0;
                    }
                    if (msgbuf.mtype == REQTYPE_MSG){
                        printf("error: %s\n", msgbuf.mtext);
                    } else {
                        connected_to_id = msgbuf.id_to_connect;
                        connected_to_qid = msgbuf.qid_to_connect;
                        printf("#Connection succeeded.\n");
                    }
                    fflush(stdout);
                }
                    break;
                case REQTYPE_LIST:
                    list_request(server_qid);
                    msg_buffer msgbuf;
                    if (msgrcv(client_qid, &msgbuf, sizeof(msgbuf), REQTYPE_NEGATIVE_LAST, NO_FLAG) == -1) {
                        perror("catch reply error");
                        return 0;
                    }
                    printf("%s", msgbuf.mtext);
                    fflush(stdout);
                    break;
                default:
                    break;
            }
            buf = 0;
            free(line);
        }
        else {
            struct msqid_ds stateb;
            msgctl(client_qid, IPC_STAT, &stateb);
            if (stateb.msg_qnum != 0) {
                msg_buffer msgbuf;
                if(msgrcv(client_qid, &msgbuf, sizeof(msgbuf), REQTYPE_NEGATIVE_LAST, IPC_NOWAIT) == -1){
                    perror("receiving msg client error");
                    continue;
                }
                switch (msgbuf.mtype){
                    case REQTYPE_CONNECT: {
                        connected_to_id = msgbuf.id_to_connect;
                        connected_to_qid = msgbuf.qid_to_connect;
                        printf("Connected to user [%d]\n", connected_to_id);
                        fflush(stdout);
                    }
                        break;
                    case REQTYPE_MSG: {
                        printf("FROM: [%d] | %s", msgbuf.sender_id, msgbuf.mtext);
                    }
                        break;
                    case REQTYPE_INIT: {
                        client_id = msgbuf.sender_id;
                        printf("#WELCOME! \n#YOUR ID: %d.\n\n", client_id);
                    }
                        break;
                    case REQTYPE_STOP: {
                        printf("#Server stopped.\n");
                        msgctl(client_qid, IPC_RMID, NULL);
                        exit(0);
                    }
                    break;
                    case REQTYPE_DISCONNECT: {
                        printf("#Disconnected.\n");
                        connected_to_qid = -1;
                        connected_to_id = -1;
                    }
                        break;
                    default:
                        break;
                }
            }
        }
        fflush(stdout);
    }
}

int list_request(int server_qid){
    msg_buffer msgbuf = {.mtype = REQTYPE_LIST, .sender_qid = client_qid};
    if (msgsnd(server_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
        perror("list request failed");
        return 0;
    }
    return 1;
}

int connect_request(int client_ID, int server_qid){
    msg_buffer msgbuf = {.mtype = REQTYPE_CONNECT, .sender_qid = client_qid, .id_to_connect = client_ID};
    if (msgsnd(server_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
        perror("connect request failed");
        return 0;
    }
    return 1;
}


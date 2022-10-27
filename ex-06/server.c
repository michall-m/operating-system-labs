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

int IDs_counter = 0;
int server_qid;
struct client clients[MAX_CONNECTED_CLIENTS];



void init_clients(){
    for(int i=0; i<MAX_CONNECTED_CLIENTS; i++){
        clients[i] = client_default;
    }
}


/*
 * REQUESTS
 */

int list_request(int client_qid);
int init_request(int client_qid);
void connect_request(int client_qid, int to_connect_ID);
int disconnect_request(int client_qid);
int stop_request(int client_qid);


void server_stop_handler();



int main(){
    if($HOME == NULL){
        perror("Can not get \"$HOME\"");
        return -1;
    }

    /*KEY AND QUEUE ID DECLARATION
     *
     */
    key_t server_key = ftok($HOME, PROJ_CHAR_ID);
    if (server_key == -1){
        perror("ftok error");
        return -1;
    }

    server_qid = msgget(server_key, IPC_CREAT | IPC_EXCL | 0666);
    if (server_qid == -1){
        perror("server qid error");
    }

    /*
     * SET: CTRL + C HANDLER & CLIENTS
     */
    signal(SIGINT, server_stop_handler);
    init_clients();



    printf("server key -> %d\nserver qid -> %d\n", server_key, server_qid);

    /*
     * MESSAGES HANDLER
     */
    while(1){
        msg_buffer msgbuf;
        if(msgrcv(server_qid, &msgbuf, sizeof(msgbuf), REQTYPE_NEGATIVE_LAST, NO_FLAG) == -1) {
            perror("server main error");
            continue;
        }
        switch(msgbuf.mtype) {
            case REQTYPE_LIST:
                printf("SERVER -> list request | sender qid -> %d\n", msgbuf.sender_qid);
                list_request(msgbuf.sender_qid);
                break;
            case REQTYPE_INIT:
                printf("SERVER -> init request | sender qid -> %d\n", msgbuf.sender_qid);
                init_request(msgbuf.sender_qid);
                break;
            case REQTYPE_CONNECT:
                printf("SERVER -> connect request | sender qid -> %d\n", msgbuf.sender_qid);
                connect_request(msgbuf.sender_qid, msgbuf.id_to_connect);
                break;
            case REQTYPE_STOP:
                printf("SERVER -> stop request | sender qid -> %d\n", msgbuf.sender_qid);
                stop_request(msgbuf.sender_qid);
                break;
            case REQTYPE_DISCONNECT:
                printf("SERVER -> disconnect request | sender qid -> %d\n", msgbuf.sender_qid);
                disconnect_request(msgbuf.sender_qid);
                break;
            default:
                break;
        }
    }
}



int list_request(int client_qid){
    msg_buffer msgbuf = {.sender_qid = server_qid, .mtype = REQTYPE_LIST};
    strcpy( msgbuf.mtext, "Connected clients (~a : available to connect): ");
    int first = 1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].ID != -1 && clients[i].queue_ID != client_qid) {
            if(first) {
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), "[%d]", clients[i].ID);
                first = 0;
            }
            else {
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), ", [%d]", clients[i].ID);
            }
            if(clients[i].connected_to == -1){
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), "~a");
            }
        }
        else if (clients[i].queue_ID == client_qid) {
            if(first) {
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), "[>%d<]", clients[i].ID);
                first = 0;
            }
            else {
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), ", [>%d<]", clients[i].ID);
            }
            if(clients[i].connected_to == -1){
                sprintf(msgbuf.mtext + strlen(msgbuf.mtext), "~a");
            }
        }
    }
    sprintf(msgbuf.mtext + strlen(msgbuf.mtext), "\n");
    printf("%s", msgbuf.mtext);
    if(msgsnd(client_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
        perror("sending list error");
        return 0;
    }
    fflush(stdout);
    return 1;
}

int stop_request(int client_qid){
    int client_index = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].queue_ID == client_qid){
            client_index = i;
            break;
        }
    }

    if(clients[client_index].connected_to != -1)  {
        msg_buffer msgbuf = {.mtype = REQTYPE_DISCONNECT};
        if (msgsnd(clients[client_index].connected_to, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1) {
            perror("server: stop request handler error");
        }
        int connected_to_index = -1;
        for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
            if(clients[i].queue_ID == clients[client_index].connected_to){
                connected_to_index = i;
                break;
            }
        }
        clients[connected_to_index].connected_to = -1;
    }

    clients[client_index] = client_default;
}

int disconnect_request(int client_qid){
    int client_index = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].queue_ID == client_qid){
            client_index = i;
            break;
        }
    }


    int connected_to_index = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].queue_ID == clients[client_index].connected_to){
            connected_to_index = i;
            break;
        }
    }
    msg_buffer msgbuf = {.mtype = REQTYPE_DISCONNECT};
    if (msgsnd(clients[connected_to_index].queue_ID, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1) {
        perror("server: disconnect  failed");
    }

    clients[client_index].connected_to = -1;
    clients[connected_to_index].connected_to = -1;

    return 1;
}

int init_request(int client_qid){
    int connected_at = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].ID == -1){
            clients[i].ID = IDs_counter++;
            clients[i].queue_ID = client_qid;
            connected_at = i;
            break;
        }
    }
    if (connected_at == -1){
        perror("Impossible to init more clients.");
        return 0;
    }
    msg_buffer msgbuf = {.mtype = REQTYPE_INIT, .sender_id = clients[connected_at].ID};
    if (msgsnd(client_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
        perror("server: init fail");
    }
    return 1;
}

void connect_request(int client_qid, int to_connect_ID){
    int to_connect_index = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].ID == to_connect_ID){
            to_connect_index = i;
            break;
        }
    }

    int client_id = -1;
    int client_index = -1;
    for(int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if(clients[i].queue_ID == client_qid){
            client_id = clients[i].ID;
            client_index = i;
            break;
        }
    }
    //nie udaje sie ->
    if(clients[client_index].connected_to != -1){
        msg_buffer msgbuf = {.mtype = REQTYPE_MSG, .mtext = "You are already connected."};
        if (msgsnd(client_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
            perror("failed to send cinf1");
        }
        return;
    }
    if (to_connect_index == -1){
        msg_buffer msgbuf = {.mtype = REQTYPE_MSG, .mtext = "Client with that ID is not connected to server."};
        if (msgsnd(client_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
            perror("failed to send cinf1");
        }
        return;
    }
    if (clients[to_connect_index].connected_to != -1){
        msg_buffer msgbuf = {.mtype = REQTYPE_MSG, .mtext = "Client with that ID is already connected with other client."};
        if (msgsnd(client_qid, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
            perror("failed to send cinf2");
        }
        return;
    }
    //udaje sie -> zwracamy jego qid i do tego o tym qid tez wysylamy tamtego qid i id
    msg_buffer msgbuf1 = {.mtype = REQTYPE_CONNECT, .id_to_connect = clients[to_connect_index].ID, .qid_to_connect = clients[to_connect_index].queue_ID};
    if (msgsnd(client_qid, &msgbuf1, sizeof(msgbuf1), NO_FLAG) == -1){
        perror("failed to send cinf3");
    }
    clients[client_index].connected_to = clients[to_connect_index].queue_ID;

    msg_buffer msgbuf2 = {.mtype = REQTYPE_CONNECT, .id_to_connect = client_id, .qid_to_connect = client_qid};
    if (msgsnd(clients[to_connect_index].queue_ID, &msgbuf2, sizeof(msgbuf2), NO_FLAG) == -1){
        perror("failed to send cinf4");
    }

    clients[to_connect_index].connected_to = client_qid;
    return;


}

void server_stop_handler(){
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
        if (clients[i].ID != -1){
            msg_buffer msgbuf = {.mtype = REQTYPE_STOP};
            if (msgsnd(clients[i].queue_ID, &msgbuf, sizeof(msgbuf), NO_FLAG) == -1){
                perror("failed to send stop signal to client");
            }
        }
    }
    msgctl(server_qid, IPC_RMID, NULL);
    exit(0);
}
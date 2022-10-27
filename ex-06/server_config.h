//
// Created by Michał Misiak on 02.05.2021.
//

#ifndef SYSOPY_SERVER_CONFIG_H
#define SYSOPY_SERVER_CONFIG_H

#define $HOME getenv("HOME")

#define REQTYPE_STOP 1
#define REQTYPE_DISCONNECT 2
#define REQTYPE_LIST 3
#define REQTYPE_CONNECT 4
#define REQTYPE_INIT 5
#define REQTYPE_MSG 6
#define REQTYPE_NEGATIVE_LAST -6

#define NO_FLAG 0

#define PROJ_CHAR_ID 'C'
#define MSG_MAX_SIZE 128
#define MAX_CONNECTED_CLIENTS 64

typedef struct msg_buffer {
    long mtype;
    char mtext[MSG_MAX_SIZE];
    int sender_qid;
    int receiver_qid;
    int id_to_connect;
    int qid_to_connect;
    int sender_id;
} msg_buffer;

struct client {
    int ID;
    int queue_ID;
    int connected_to;
} client_default = { .connected_to=-1, .ID=-1, .queue_ID = -1};



#endif //SYSOPY_SERVER_CONFIG_H

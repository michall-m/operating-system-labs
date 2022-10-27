#ifndef ZAD01_COMMON_H
#define ZAD01_COMMON_H


#define MSG_MAX_LEN 64
#define MAX_N_OF_PLAYERS 16

#define MSG_TYPE_MOVE '0'
#define MSG_TYPE_CONNECT '1'
#define MSG_TYPE_PING '2'
#define MSG_TYPE_DISCONNECT '3'

typedef struct server_s {
    int socket;
    char* address;
} server_s;

void notify_server(char msg_type, char* msg_content, server_s server) {
    char smsg[MSG_MAX_LEN];
    smsg[0] = msg_type;
    for (int i = 0; i < strlen(msg_content); i++) {
        smsg[i+1] = msg_content[i];
    }
    smsg[strlen(msg_content)+1] = '\0';
    send(server.socket, smsg, MSG_MAX_LEN, 0);
}

char* msg_content(char* msg) {
    return msg+1;
}



#endif //ZAD01_COMMON_H

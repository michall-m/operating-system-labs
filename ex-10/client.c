#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "board.h"
#include "common.h"

server_s server;
char* client_name;
bool is_connected = false;
board_s* board;
player_t clients_player_type = PLAYER_UNDEFINED;

int connect_local();
int connect_network();
void handle_connection();

pthread_mutex_t mutex;
pthread_cond_t cond;

/*
 * destination -> server.address
 *
 *
 *
 *
 *
 *
 */
int main(int argc, char** argv) {
    if (argc-1 != 3) {
        printf("Podaj poprawne argumenty: nazwa uzytkownika, sposób polaczenia, adres serwera");
    }
    client_name             = argv[1];
    char* connection_type   = argv[2];
    server.address          = argv[3];

    if (strcmp(connection_type, "network")) {
        connect_network();
    } else if (strcmp(connection_type, "local")) {
        connect_local();
    } else {
        printf("Invalid value of connection_type argument.");
        exit(EXIT_FAILURE);
    }

    handle_connection();
}


int connect_local() {
    struct sockaddr_un sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_un)); //TODO size of sockaddr
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, server.address);

    server.socket = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(server.socket, &sockaddr, sizeof(struct sockaddr_un));

    return 1;
}

int connect_network() {
    //TODO all
    struct addrinfo *info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo("localhost", server.address, &hints, &info);
    server.socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    connect(server.socket, info->ai_addr, info->ai_addrlen);
    freeaddrinfo(info);
}


void handle_connection() {
    notify_server(MSG_TYPE_CONNECT, client_name, server);

    char msg_recv[MSG_MAX_LEN];

    while(true) {
        recv(server.socket, msg_recv, MSG_MAX_LEN, 0);

        switch (msg_recv[0]) {
            case MSG_TYPE_CONNECT: {
                if (is_connected) {
                    printf("User is already connected.\n");
                }
                board = malloc(sizeof(board_s));
                *board = get_clean_board();
                if (msg_recv[1] == 'O') {
                    clients_player_type = PLAYER_O;
                } else if (msg_recv[1] == 'X') {
                    clients_player_type = PLAYER_X;
                } else {
                    printf("Invalid player type arg.\n");
                    exit(EXIT_FAILURE);
                }
            }
                break;
            case MSG_TYPE_DISCONNECT: {
                exit(EXIT_SUCCESS);
            }
                break;
            case MSG_TYPE_MOVE: {
                print_out(board);

                int m = board->filled_squares;
                int position;
                do {
                    scanf("%d", &position);
                    try_to_move(board, position);
                } while (board->filled_squares == m);

                print_out(board);
                char n[1];
                n[0] = '0' + position;
                notify_server(MSG_TYPE_MOVE, n, server);
            }
                break;

        }
    }

}
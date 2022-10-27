#ifndef SYSOPY_BOARD_H
#define SYSOPY_BOARD_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <stdbool.h>



typedef enum square_t {
    SQ_UNOCCUPIED=0,
    SQ_X=1,
    SQ_O=2
} square_t;

typedef enum player_t {
    PLAYER_UNDEFINED=0,
    PLAYER_X=1,
    PLAYER_O=2
} player_t;

typedef struct board_s {              // [1] [2] [3]
    square_t squares[9];            // [4] [5] [6]
    player_t next_move;             // [7] [8] [9]
    bool is_the_game_over;
    player_t winner;
    short int filled_squares;
} board_s;

board_s get_clean_board() {
    return (struct board_s) { .squares={[0 ... 8]=SQ_UNOCCUPIED},
                            .next_move=PLAYER_O,
                            .is_the_game_over=false,
                            .winner = PLAYER_UNDEFINED,
                            .filled_squares=0};
}

//methods
int try_to_move(board_s *b, int square_number) {
    if (square_number < 1 || square_number > 9) {
        return 0;
    }
    if (b->squares[square_number-1]  != SQ_UNOCCUPIED) {
        return 0;
    }
    switch (b->next_move) {
        case PLAYER_X:
            b->squares[square_number-1] = SQ_X;
            b->next_move = PLAYER_O;
            break;
        case PLAYER_O:
            b->squares[square_number-1] = SQ_O;
            b->next_move = PLAYER_X;
            break;
        default:
            printf("try_to_move failed.");
            exit(EXIT_FAILURE);
    }
    b->filled_squares++;

    //winner
    if (b->filled_squares<5) {
        return 1;
    } else {
        //columns
        for (int j = 0; j < 3; j++) {
            if(b->squares[j] == SQ_O && b->squares[j+3] == SQ_O && b->squares[j+6] == SQ_O) {
                b->winner = PLAYER_O;
                b->is_the_game_over = true;
            } else if (b->squares[j] == SQ_X && b->squares[j+3] == SQ_X && b->squares[j+6] == SQ_X) {
                b->winner = PLAYER_X;
                b->is_the_game_over = true;
            }
        }
        //rows
        for (int i = 0; i <=6 ; i += 3) {
            if(b->squares[i] == SQ_O && b->squares[i+1] == SQ_O && b->squares[i+2] == SQ_O) {
                b->winner = PLAYER_O;
                b->is_the_game_over = true;
            } else if (b->squares[i] == SQ_X && b->squares[i+1] == SQ_X && b->squares[i+2] == SQ_X) {
                b->winner = PLAYER_X;
                b->is_the_game_over = true;
            }
        }
        //diagonals
        if(b->squares[0]==b->squares[4] && b->squares[4] == b->squares[8]) {
            if(b->squares[0] == SQ_O) {
                b->winner = PLAYER_O;
                b->is_the_game_over = true;
            } else if (b->squares[0] == SQ_X) {
                b->winner = PLAYER_X;
                b->is_the_game_over = true;
            }
        } else if (b->squares[2]==b->squares[4] && b->squares[4]==b->squares[6]) {
            if(b->squares[2] == SQ_O) {
                b->winner = PLAYER_O;
                b->is_the_game_over = true;
            } else if (b->squares[2] == SQ_X){
                b->winner = PLAYER_X;
                b->is_the_game_over = true;
            }
        }
    }

    return 1;
}

void print_out(board_s *b) {
    printf("[ ");
    for (int i = 0; i < 9; i++) {
        switch (b->squares[i]) {
            case SQ_UNOCCUPIED:
                printf("%d", i+1);
                break;
            case SQ_X:
                printf("X");
                break;
            case SQ_O:
                printf("O");
                break;
        }
        printf("]");
        if ((i+1)%3 == 0) {
            printf("\n");
        }
        else {
            printf(" ");
        }
    }
}

#endif //SYSOPY_BOARD_H

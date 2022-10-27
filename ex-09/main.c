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
#include <pthread.h>

#define NO_REINDEERS 9
#define NO_ELVES 10
#define MAX_WAITING_ELVES 3

typedef enum sleep_t {
    S_ELF_WORKS=0,
    S_SANTA_SOLVES_PROBLEM=1,
    S_REINDEER_REST=2,
    S_GIFTING=3,
} sleep_t;

typedef enum christmas_mutex_t {
    M_SANTA_SLEEPING=0,
    M_REINDEERS_WAITING=1,
    M_REINDEERS_GIFTING=2,
    M_ELVES_WAITING=3,
    M_ELVES_WORKING=4
} christmas_mutex_t;

typedef enum christmas_conds_t {
    C_SANTA=0,
    C_REINDEERS=1,
    C_ELVES=2
} christmas_conds_t;

typedef enum mutex_op_t {
    MOP_LOCK=0,
    MOP_UNLOCK=1
} mutex_op_t;

void wait_rand(sleep_t sleep);
void try_mutex_op(pthread_mutex_t* mutex, mutex_op_t mop);


pthread_mutex_t mutexes[5]={[0 ... 4]=PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t conds[3]={[0 ... 2]=PTHREAD_COND_INITIALIZER};
int reindeers_available=0;
int giftings_left = 3;
int elves_having_problems=0;
int elf_ids[NO_ELVES] = {0,1,2,3,4,5,6,7,8,9};
pthread_t elf_pthreads[NO_ELVES];
int waiting_elves[MAX_WAITING_ELVES];
int reindeer_ids[NO_REINDEERS] = {0,1,2,3,4,5,6,7,8};
pthread_t reindeer_pthreads[NO_REINDEERS];
pthread_t santa_pthread;

void* santa_f(void* arg);
void* reindeer_f(void* arg);
void* elf_f(void* arg);

int main(int argc, char** argv) {
    pthread_create(&santa_pthread, NULL, &santa_f, (void *) NULL);
    for (int i = 0; i < NO_ELVES; i++) {
        pthread_create(&elf_pthreads[i], NULL, &elf_f, &elf_ids[i]);
    }
    for (int i = 0; i < NO_REINDEERS; i++) {
        pthread_create(&reindeer_pthreads[i], NULL, &reindeer_f, &reindeer_ids[i]);
    }

    pthread_join(santa_pthread, (void**) NULL);
    printf("Mikołaj uznał święta za zakończone.\n");

    exit(EXIT_SUCCESS);
}

void* santa_f(void* arg) {
    while(giftings_left>0) {
        try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_LOCK);
        while(reindeers_available < NO_REINDEERS && elves_having_problems < MAX_WAITING_ELVES) {
            pthread_cond_wait(&conds[C_SANTA], &mutexes[M_SANTA_SLEEPING]);
        }
        try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_UNLOCK);
        printf("Mikołaj: budzę się.\n");

        try_mutex_op(&mutexes[M_REINDEERS_GIFTING], MOP_LOCK);
        if (reindeers_available == NO_REINDEERS) {
            printf("Mikołaj: dostarczam zabawki.\n");
            wait_rand(S_GIFTING);
            giftings_left--;
            printf("Mikołaj: pozostałe tury: %d.\n",giftings_left);
            try_mutex_op(&mutexes[M_REINDEERS_WAITING], MOP_LOCK);
            reindeers_available=0;
            pthread_cond_broadcast(&conds[C_REINDEERS]);
            try_mutex_op(&mutexes[M_REINDEERS_WAITING], MOP_UNLOCK);
        }
        try_mutex_op(&mutexes[M_REINDEERS_GIFTING], MOP_UNLOCK);

        try_mutex_op(&mutexes[M_ELVES_WORKING], MOP_LOCK);
        if (elves_having_problems == MAX_WAITING_ELVES) {
            printf("Mikołaj: rozwiązuje problemy elfów o ID: |%d_%d_%d|\n", waiting_elves[0],
                    waiting_elves[1], waiting_elves[2]);
            wait_rand(S_SANTA_SOLVES_PROBLEM);
            try_mutex_op(&mutexes[M_ELVES_WAITING], MOP_LOCK);
            elves_having_problems=0;
            pthread_cond_broadcast(&conds[C_ELVES]);
            try_mutex_op(&mutexes[M_ELVES_WAITING], MOP_UNLOCK);
        }
        try_mutex_op(&mutexes[M_ELVES_WORKING], MOP_UNLOCK);

        printf("Mikołaj: zasypiam.\n");
    }
    for (int i = 0; i < NO_ELVES; ++i) {
        pthread_cancel(elf_pthreads[i]);
    }
    for (int i = 0; i < NO_REINDEERS; ++i) {
        pthread_cancel(reindeer_pthreads[i]);
    }
}

void* elf_f(void* arg) {
    while(giftings_left>0) {
        wait_rand(S_ELF_WORKS);

        try_mutex_op(&mutexes[M_ELVES_WAITING], MOP_LOCK);
        while (elves_having_problems >= MAX_WAITING_ELVES) {
            printf("Elf: czeka na powrót elfów, |%d|.\n", *((int *) arg));
            pthread_cond_wait(&conds[C_ELVES], &mutexes[M_ELVES_WAITING]);
        }
        try_mutex_op(&mutexes[M_ELVES_WAITING], MOP_UNLOCK);

        try_mutex_op(&mutexes[M_ELVES_WORKING], MOP_LOCK);
        if (elves_having_problems < MAX_WAITING_ELVES) {
            waiting_elves[elves_having_problems++] = *((int *) arg);
            printf("Elf: czeka %d elfów na mikołaja, |%d|\n", elves_having_problems,
                    waiting_elves[elves_having_problems-1]);
            if(elves_having_problems >= MAX_WAITING_ELVES) {
                try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_LOCK);
                if(giftings_left == 0) {
                    continue;
                }
                printf("Elf: wybudzam mikołaja, |%d|.\n", waiting_elves[MAX_WAITING_ELVES-1]);
                pthread_cond_broadcast(&conds[C_SANTA]);
                try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_UNLOCK);
            }
        }
        try_mutex_op(&mutexes[M_ELVES_WORKING], MOP_UNLOCK);
    }
}


void* reindeer_f(void* arg) {
    while(giftings_left>0) {
        try_mutex_op(&mutexes[M_REINDEERS_WAITING], MOP_LOCK);
        while (reindeers_available > 0) {
            pthread_cond_wait(&conds[C_REINDEERS], &mutexes[M_REINDEERS_WAITING]);
        }
        try_mutex_op(&mutexes[M_REINDEERS_WAITING], MOP_UNLOCK);

        wait_rand(S_REINDEER_REST);

        try_mutex_op(&mutexes[M_REINDEERS_GIFTING], MOP_LOCK);
        printf("Renifer: czeka %d reniferów na Mikołaja, %d.\n", ++reindeers_available, *((int *) arg));
        if (reindeers_available == NO_REINDEERS) {
            try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_LOCK);
            if(giftings_left == 0) {
                continue;
            }
            printf("Renifer: wybudzam Mikołaja, %d.\n", *((int *) arg));
            pthread_cond_broadcast(&conds[C_SANTA]);
            try_mutex_op(&mutexes[M_SANTA_SLEEPING], MOP_UNLOCK);
        }
        try_mutex_op(&mutexes[M_REINDEERS_GIFTING], MOP_UNLOCK);
    }
}



void wait_rand(sleep_t sleep){
    int seconds;
    switch (sleep) {
        case S_ELF_WORKS: {
            seconds = rand()%3 + 2;
        } break;
        case S_SANTA_SOLVES_PROBLEM: {
            seconds = 1;
        } break;
        case S_REINDEER_REST: {
            seconds = rand()%5 + 5;
        } break;
        case S_GIFTING: {
            seconds = rand()%2 + 2;
        } break;
        default: {
            printf("wait_rand invalid arg.");
            exit(EXIT_FAILURE);
        }
    }

    struct timespec ts = {
            .tv_sec = seconds,
            .tv_nsec = (rand() % 10000) * 10000
    };
    nanosleep(&ts, NULL);
}

void try_mutex_op(pthread_mutex_t* mutex, mutex_op_t mop) {
    switch (mop) {
        case MOP_LOCK: {
            if (pthread_mutex_lock(mutex) != 0) {
                perror("Mutex locking error ");
                exit(EXIT_FAILURE);
            }
        }
            break;
        case MOP_UNLOCK: {
            if (pthread_mutex_unlock(mutex) != 0) {
                perror("Mutex unlocking error ");
                exit(EXIT_FAILURE);
            }
        }
            break;
        default:
            printf("Invalid mutex_op_t val.");
            break;
    }
    return;
}

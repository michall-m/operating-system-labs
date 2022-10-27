/*
WAP to delete all message queues?
you can delete message queue's using KEY number also(use system(), but message Queue with KEY=0, cannot be removed in that way ->" ipcrm -Q 0 " not possible, better is get mesQid for each key, then use msgctl(msgQid,IPC_RMID,0);
in this way you can remove with key=0 also;
need to get msgqid by avoiding msgget() -> overhead but, worth doing
read MSG_INFO and MSG_STAT from $man 2 msgctl
Author	-	Chetan Naik
mailId	-	chetandevanaik@gmail.com
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    char* line;
    size_t buf = 0;
    FILE *qids = popen("ipcs -a", "r");
    while(getline(&line, &buf, qids) > 0){
        printf("%s", line);
    }
}
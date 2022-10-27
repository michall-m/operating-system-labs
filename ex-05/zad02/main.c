#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char** argv){
    if(argc!= 2 && argc != 4){
        printf("Argumenty programu powinny być w jednym z poniższych formatów:\n");
        printf("[nadawca/data]\n");
        printf("<adresEmail> <tytuł> <treść>\n");
        return EXIT_FAILURE;
    }
    if (argc == 2){
        FILE* mailbox;
        if(strcmp(argv[1], "nadawca") == 0){
            mailbox = popen("mail -H | sort -f -k 3 | grep -i @", "r");
        }
        else if(strcmp(argv[1], "data") == 0){
            mailbox = popen("mail -H | grep -i @ | sort -k5,1M -k6 -k7", "r");
        }
        else {
            printf("Niepoprawny argument.\n");
        }
        char* mail_info;
        size_t buff = 0;
        while(getline(&mail_info, &buff, mailbox) > 0){
            printf("%s", mail_info);
            buff = 0;
        }
        pclose(mailbox);
    }
    if (argc == 4){
        char* command = malloc(1000 * sizeof(char));
        sprintf(command, "echo %s | mail -s %s %s", argv[3], argv[2], argv[1]);
        FILE* mailbox = popen(command, "r");
        free(command);
        pclose(mailbox);
    }
}
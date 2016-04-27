#include "signal.h"
#include "unistd.h"
#include "stdio.h"

volatile sig_atomic_t flag = 0;

void signalHandler(int signum){
    flag = signum;
}

int main(int argc, char* argv[]){

    int busy_wait = 0;
    int loop = 1;

    if (argc > 2){
        perror("Too many aruments. Type --help for usage");
        return -1;
    }
    else if (argc == 2){
        if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
            printf("Usage: ./receiver --busy (for busy wait loop)\n");
            printf("       ./receiver (for blocking wait loop)\n");
        }
        else if (!strcmp(argv[1], "--busy")){
            busy_wait = 1;
        }
    }

    // Assign signal handling function to SIGINT
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);

    while (loop){
        if (!busy_wait){
            pause();
        }
        if (flag){
            switch(flag){
                case 2:
                    printf("You can't interrupt me. :)\n");
                    break;
                case 3:
                    printf("Terminating program... Bye!\n");
                    loop = 0;
                    break;
                case 10:
                    printf("User action 1!\n");
                    break;
                case 12:
                    printf("User action 2!\n");
                    break;
            }
            flag = 0;
        }
    }
    return 0;
}
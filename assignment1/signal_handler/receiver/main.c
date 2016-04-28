#include "signal.h"
#include "unistd.h"
#include "stdio.h"

volatile sig_atomic_t flag = 0;

// Signal handler funtion, linked with all signals
void signalHandler(int signum){
    flag = signum;
}

int main(int argc, char* argv[]){

    int busy_wait = 0;
    int loop = 1;

    // Check for wrong number of arguments
    if (argc > 2){
        perror("Too many aruments. Type --help for usage");
        exit(1);
    }
    else if (argc == 2){
        // Help and proper treatment
        if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
            printf("Usage: ./app_receiver --busy (for busy wait loop)\n");
            printf("       ./app_receiver (for blocking wait loop)\n");
            return 0;
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
        // Busy wait
        if (!busy_wait){
            pause();
        }
        // Do things for signal handling since printf is bad
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
            // Reset flag
            flag = 0;
        }
    }
    return 0;
}
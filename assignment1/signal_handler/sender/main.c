#include "signal.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char* argv[]){

    // Help
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./sender <process_id [int]> <signal [int]>\n");
        return 0;
    }

    // Check if there are 2 arguments given
    if (argc != 3){
        perror("Wrong number of arguments. Type --help for usage");
        return -1;
    }

    // Convert p_id and signal to integers
    int p_id = atoi(argv[1]);
    int sig = atoi(argv[2]);

    // Send signal
    int ok = kill(p_id, sig);

    // Print OK or error message 
    if (ok){
        perror("Could not send signal to process");
        return -1;
    }
    else{
        printf("Signal %i sent to process %i\n", sig, p_id);
    }

    return 0;
}
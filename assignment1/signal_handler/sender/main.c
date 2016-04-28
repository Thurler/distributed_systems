#include "signal.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char* argv[]){

    int p_id, sig, ok;

    // Help
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./app_sender <process_id [int]> <signal [int]>\n");
        return 0;
    }

    // Check if there are 2 arguments given
    if (argc != 3){
        perror("Wrong number of arguments. Type --help for usage");
        exit(1);
    }

    // Convert p_id and signal to integers
    p_id = atoi(argv[1]);
    sig = atoi(argv[2]);

    // Send signal
    ok = kill(p_id, sig);

    // Print OK or error message 
    if (ok){
        perror("Could not send signal to process");
        exit(1);
    }
    else{
        printf("Signal %i sent to process %i\n", sig, p_id);
    }

    return 0;
}
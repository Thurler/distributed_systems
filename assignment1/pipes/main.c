#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

#define MAXSTEP 10
#define BUFSIZE 11

int isprime(int n){
    int i;
    for (i = 2; i <= n/2; i++){
        if (n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char const *argv[]){

    // Set variables and rand seed
    srand(time(NULL));
    unsigned int count = 0;
    int fd[2], i, n, val;
    double random;
    pid_t childpid;
    char number[BUFSIZE];
    char readbuffer[BUFSIZE];

    //Check for arguments - too few/many, help and proper handling
    if (argc != 2){
        perror("Arguments specified wrongly. Type --help for usage");
        exit(1);
    }
    else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
        printf("Usage: ./app_pipes <number of numbers to generate>\n");
        return 0;
    }
    else{
        n = atoi(argv[1]);
    }

    // Create pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork process
    if((childpid = fork()) == -1){
            perror("fork");
            return -1;
    }

    if (!childpid){
        // This segment is run by child process
        // Close write port
        close(fd[1]);
        // Loop while zero isnt read
        while (read(fd[0], readbuffer, sizeof(readbuffer)) > 0){
            printf("Child received %s!\n", readbuffer);
            if (readbuffer[0] == '0'){
                // Close pipe, exit
                close(fd[0]);
                exit(0);
            }
            //Check if prime or not
            val = atoi(readbuffer);
            if (isprime(val)){
                printf("%i is a prime number!\n", val);
            }
            else{
                printf("%i is not a prime number!\n", val);
            }
        }
    }

    else{
        // This segment is run by parent process
        // Close read port
        close(fd[0]);
        // Loop for n
        for (i = 0; i < n; i++){
            random = (double)rand() / (double)RAND_MAX;
            count += 1 + (int)(MAXSTEP * random);
            sprintf(number, "%i", count);
            printf("Parent sending %s...\n", number);
            write(fd[1], number, BUFSIZE);
        }
        // Send zero
        printf("Parent sending 0...\n");
        write(fd[1], "0", BUFSIZE);
        // Close pipe
        close(fd[1]);
    }

    return 0;
}
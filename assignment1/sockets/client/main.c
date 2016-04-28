#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXSTEP 10
#define BUFSIZE 11

int main(int argc, char *argv[]){

    // Initialize variables and set rand seed
    srand(time(NULL));
    unsigned int count = 0;
    int sockfd, portn, i, n, numbers;
    double random;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[256];

    // Help
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./app_client <server> <port> <number of numbers to generate>\n");
        return 0;
    }

    // Check if server and port specified
    if (argc != 4){
        perror("Arguments specified wrongly. See --help for usage.");
        exit(1);
    }
    else{
        // Get host based on command line
        server = gethostbyname(argv[1]);
        if (server == NULL) {
            fprintf(stderr,"Error, no such host\n");
            exit(0);
        }

        // Get port number
        portn = atoi(argv[2]);

        // Get number of numbers to generate
        numbers = atoi(argv[3]);
    }

    // Open socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error opening socket");
        exit(1);
    }

    // Clear server address buffer
    bzero((char *) &server_addr, sizeof(server_addr));

    // Set address attributes
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portn);

    // Connect to host
    if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0){
        perror("Error connecting to host");
        exit(1);
    }

    for (i = 0; i < numbers; i++){
        // Clean buffer
        bzero(buffer,256);

        // Generate random number
        random = (double)rand() / (double)RAND_MAX;
        count += 1 + (int)(MAXSTEP * random);
        sprintf(buffer, "%i", count);
        printf("Client sending %s...\n", buffer);

        // Send message to host
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0){
            error("Error writing to socket");
            exit(1);
        }

        // Clean buffer, receive reply from host
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0){
            perror("Error reading from socket");
            exit(1);
        }

        // Print reply on screen
        printf("%s",buffer);
    }

    // Clean buffer
    bzero(buffer,256);

    sprintf(buffer, "0");
    printf("Client sending %s...\n", buffer);

    // Send message to host
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){
        error("Error writing to socket");
    }

    // Close client once all numbers are sent
    close(sockfd);

    return 0;
}
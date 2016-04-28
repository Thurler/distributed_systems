#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int isprime(int n){
    int i;
    for (i = 2; i <= n/2; i++){
        if (n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]){

    // Initialize variables
    int sockfd, newsockfd, portn, val;
    socklen_t clientlen;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;
    int n;

    // Check wrong number of arguments, help and handle input
    if (argc != 2){
        perror("Arguments specified wrongly. Type --help for usage");
        exit(1);
    }
    else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
        printf("Usage: ./app_server <port>\n");
        return 0;
    }
    else{
        portn = atoi(argv[1]);
    }

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating socket");
        exit(1);
    }

    // Clean server address buffer
    bzero((char *) &server_addr, sizeof(server_addr));

    // Set server address attributes
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portn);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("Error on binding");
        exit(1);
    }

    // Set server to listen
    listen(sockfd,5);

    // Get size of client address
    clientlen = sizeof(client_addr);

    // Open new socket for communication with client, accept request
    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);
    if (newsockfd < 0){
        perror("Error on accept");
        exit(1);
    }

    while (1){
        // Loop while the client sends things

        // Clean buffer, read message
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0){
            perror("Error reading from socket");
        }

        printf("Received %s from client...\n", buffer);

        // Check if zero sent, break out of loop and terminate if so
        if (buffer[0] == '0'){
            break;
        }

        // Cast message to number
        val = atoi(buffer);

        // Check if prime, reply accordingly
        bzero(buffer,256);
        if (isprime(val)){
            sprintf(buffer, "%i is a prime number!\n", val);
        }
        else{
            sprintf(buffer, "%i is not a prime number!\n", val);
        }

        n = write(newsockfd,buffer,strlen(buffer));
        if (n < 0){
            error("Error writing to socket");
        }
    }

    close(newsockfd);
    close(sockfd);

    return 0;
}
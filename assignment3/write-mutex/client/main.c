#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

#define REQUEST 0
#define RELEASE 1
#define GRANT 2
#define BYE 9
#define WRITES 100

int main(int argc, char *argv[]){

    // Initialize variables
    FILE *pFile;
    unsigned int i, retry, slptime;
    int sockfd, portn, n, msg, id;
    double random;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[16];

    // Help
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./app_client <server> <port> <id>\n");
        return 0;
    }

    // Check if server and port specified
    if (argc != 4){
        printf("Arguments specified wrongly. See --help for usage.\n");
        exit(1);
    }
    else{
        // Get host based on command line
        server = gethostbyname(argv[1]);
        if (server == NULL) {
            printf("Error, no such host\n");
            exit(0);
        }

        // Get port number
        portn = atoi(argv[2]);

        // Get program id
        id = atoi(argv[3]);
    }

    srand(time(NULL)+id);

    // Open socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error opening socket\n");
        exit(1);
    }

    // Clear server address buffer
    bzero((char *) &server_addr, sizeof(server_addr));

    // Set address attributes
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portn);

    // Connect to host - retry 10 times if failure
    retry = 0;
    while (true) {
        // Try to connect
        if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0){
            retry++;
        }
        else {
            break;
        }

        if (retry > 10) {
            printf("Error connecting to host\n");
            exit(1);
        }

        // Sleep for random time, retry 0-1 second
        slptime = rand();
        random = (1000000.0f * slptime) / RAND_MAX;
        slptime = (unsigned int) random;
        usleep(slptime);
    }

    // Write to file - append line
    for (i = 0; i < WRITES; i++){
        // Clean buffer
        bzero(buffer,16);

        // Send request for lock
        sprintf(buffer, "%i", REQUEST);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0){
            printf("Error writing to socket\n");
            exit(1);
        }

        printf("Waiting reply...\n");

        // Clean buffer, receive reply from host
        bzero(buffer,16);
        n = read(sockfd, buffer, 15);
        if (n < 0){
            printf("Error reading from socket\n");
            exit(1);
        }
        msg = atoi(buffer);

        printf("Received reply!\n");

        // Write to file
        if (msg == GRANT) {
            pFile = fopen("mutex-write.txt", "a");
            fprintf(pFile, "Hello #%u from process %i!\n", i, id);
            fclose(pFile);
        }
        else {
            printf("Message received not understood\n");
            exit(1);
        }

        printf("Wrote to file!\n");

        // Clean buffer
        bzero(buffer, 16);

        //Send release for lock
        sprintf(buffer, "%i", RELEASE);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0){
            printf("Error writing to socket\n");
            exit(1);
        }

        printf("Sending release...\n");

        //Sleep for random time 0-1 second
        slptime = rand();
        random = (1000000.0f * slptime) / RAND_MAX;
        slptime = (unsigned int) random;
        usleep(slptime);
    }

    printf("hi\n");

    // Clean buffer
    bzero(buffer,16);

    // Send bye to server
    sprintf(buffer, "%i", BYE);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){
        error("Error writing to socket");
    }

    // Close client once all numbers are sent
    close(sockfd);

    return 0;
}

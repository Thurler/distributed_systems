#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXCLIENT 128
#define REQUEST 0
#define RELEASE 1
#define GRANT 2
#define BYE 9
#define WRITES 100

typedef struct linkedlist_elem linkedlist_elem;

struct linkedlist_elem {
    int value;
    linkedlist_elem *next;
};

typedef struct linkedlist {
    linkedlist_elem *head;
    linkedlist_elem *tail;
} linkedlist;

int main(int argc, char *argv[]){

    // Initialize variables
    bool lock = false;
    fd_set sockfdset;
    unsigned int i, connected_clients;
    int clientsockets[MAXCLIENT];
    int sockfd, newsockfd, selectedfd, activity, portn, n, max_sd, msg;
    socklen_t clientlen;
    char buffer[16];
    struct sockaddr_in server_addr, client_addr;

    // Initialize lock queue
    linkedlist lock_queue;
    lock_queue.head = NULL;

    // Check wrong number of arguments, help and handle input
    if (argc != 2){
        printf("Arguments specified wrongly. Type --help for usage\n");
        exit(1);
    }
    else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
        printf("Usage: ./app_server <port>\n");
        return 0;
    }
    else{
        portn = atoi(argv[1]);
    }

    // Set client sockets to zero
    for (i = 0; i < MAXCLIENT; i++) {
        clientsockets[i] = 0;
    }

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error creating socket\n");
        exit(1);
    }

    //Set socket to allow multiple connections
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        printf("Error changing socket options\n");
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
        printf("Error on binding\n");
        exit(1);
    }

    // Set server to listen
    listen(sockfd,1);

    // Get size of client address
    clientlen = sizeof(client_addr);

    // Set connected clients to zero
    connected_clients = 0;

    while (1) {
        // Clear socket set
        FD_ZERO(&sockfdset);

        // Add master socket to set
        FD_SET(sockfd, &sockfdset);
        max_sd = sockfd;

        // Add other sockets to set
        for (i = 0; i < MAXCLIENT; i++) {
            selectedfd = clientsockets[i];
            if (selectedfd > 0) {
                FD_SET(selectedfd, &sockfdset);
                if (selectedfd > max_sd) {
                    max_sd = selectedfd;
                }
            }
        }

        // Wait for an activity on one of the sockets
        activity = select(max_sd + 1, &sockfdset, NULL, NULL, NULL);

        // Check if return is valid
        if (activity < 0) {
            printf("Error on select\n");
            exit(1);
        }

        // Incoming connection
        if (FD_ISSET(sockfd, &sockfdset)) {
            newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);
            if (newsockfd < 0) {
                printf("Error on accept\n");
                continue;
            } 

            // Add new socket to array of sockets
            clientsockets[connected_clients] = newsockfd;
            connected_clients++;

            // Add new socket to set
            FD_SET(newsockfd, &sockfdset);
            printf("Established connection to new client\n");
            continue;
        }

        // Incoming message from someone - check who it is
        for (i = 0; i < MAXCLIENT; i++) {
            selectedfd = clientsockets[i];
            if (selectedfd > 0 && FD_ISSET(selectedfd, &sockfdset)) {
                // Clean buffer and read message
                bzero(buffer,16);
                n = read(selectedfd, buffer, 15);
                if (n < 0){
                    printf("Error reading from socket\n");
                    continue;
                }
                msg = atoi(buffer);
                linkedlist_elem *e;
                switch(msg) {
                case REQUEST:
                    // Accept request
                    if (lock_queue.head == NULL) {
                        // No one in queue, take lock
                        e = (linkedlist_elem *) malloc(sizeof(linkedlist_elem));
                        e->value = i;
                        e->next = NULL;
                        lock_queue.head = e;
                        lock_queue.tail = e;
                        lock = true;
                        // Send GRANT back to client
                        bzero(buffer,16);
                        sprintf(buffer, "%i", GRANT);
                        n = write(selectedfd, buffer, strlen(buffer));
                        if (n < 0){
                            printf("Error writing to socket\n");
                            exit(1);
                        }
                    }
                    else {
                        // Lock taken, join queue
                        e = (linkedlist_elem *) malloc(sizeof(linkedlist_elem));
                        e->value = i;
                        e->next = NULL;
                        lock_queue.tail->next = e;
                        lock_queue.tail = e;
                    }
                    break;
                case RELEASE:
                    // Accept release
                    e = lock_queue.head;
                    lock_queue.head = e->next;
                    free(e);
                    if (lock_queue.head == NULL) {
                        printf("hi\n");
                        lock_queue.tail = NULL;
                        lock = false;
                    }
                    else {
                        // Send GRANT back to next client
                        e = lock_queue.head;
                        bzero(buffer,16);
                        sprintf(buffer, "%i", GRANT);
                        n = write(clientsockets[e->value], buffer, strlen(buffer));
                        if (n < 0){
                            printf("Error writing to socket\n");
                            exit(1);
                        }
                    }
                    break;
                case BYE:
                    // Disconnect socket
                    FD_CLR(selectedfd, &sockfdset);
                    close(selectedfd);
                    // Update variables
                    clientsockets[i] = 0;
                    connected_clients--;
                    break;
                }
                break;
            }
        }
    }

    return 0;
}
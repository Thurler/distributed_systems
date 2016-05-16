#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define RANDOMS 10000
#define RMAX 10000000

// Information to be passed along to each thread
typedef struct{
    size_t thread_id; // Thread ID
} thread_data;

// Producer thread function
void *produce(void *arg){
    pthread_exit(NULL);
}

// Consumer thread function
void *consume(void *arg){
    pthread_exit(NULL);
}

// Generate a random number
size_t genrandom(){

    // Generate number between 1 and RMAX - 1
    double r = (double)rand() / (double)RAND_MAX;
    r *= RMAX - 1;
    r += 1;

    return (size_t)r;
}

int main(int argc, char *argv[]){

    srand(time(NULL));

    size_t size, producers, consumers;

    // Parse arguments
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./app_semaphores <N> <P> <C>\n"
               "       N: Number of numbers in shared memory\n"
               "       P: Number of producers\n"
               "       C: Number of consumers\n");
        exit(0);
    }

    else if (argc > 4 || argc < 2){
        printf("Arguments specified wrongly. Use --help for usage.\n");
        exit(-1);
    }

    // Get number of positions in shared memory
    size = atoi(argv[1]);

    // Get number of producer threads
    producers = atoi(argv[2]);

    // Get number of consumer threads
    consumers = atoi(argv[3]);

    // Configure pthread - number of threads
    size_t n_threads = producers + consumers;
    pthread_t threads[n_threads];

    // Start thread data array - on heap so threads can access it
    thread_data *thread_data_array;
    thread_data_array = (thread_data *)malloc(sizeof(thread_data) * n_threads);
    if (!thread_data_array){
        perror("Not enough memory");
        exit(-1);
    }

    // Configure thread attributes - force to be joinable
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Start variable that will act as shared memory - in heap
    size_t *shared;
    shared = (size_t *)malloc(sizeof(size_t) * size);
    size_t i;
    for (i = 0; i < size; i++){
        shared[i] = 0;
    }

    // Compute individual thread load and remainder in case of uneven division
    size_t load, rest;
    load = RANDOMS / n_threads;
    rest = RANDOMS - (load * n_threads);

    return 0;
}
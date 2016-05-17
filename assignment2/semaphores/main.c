#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define RANDOMS 10000
#define RMAX 10000000
#define EMPTY 0

sem_t semRead; // Controls read access - starts at ZERO
sem_t semWrite; // Controls write access - starts at SIZE

// Control read access - prevents two readers from reading same position
pthread_mutex_t mutexRead;
pthread_mutex_t mutexWrite;
// Control write access - prevents two writers from writing on same position

// Information to be passed along to each thread
typedef struct{
    size_t thread_id; // Thread ID
    size_t iterations; // Number of numbers to treat
    size_t *shared; // Shared memory array
    size_t size; // Size of shared memory
} thread_data;

// Generate a random number
size_t genrandom(){

    // Generate number between 1 and RMAX - 1
    double r = (double)rand() / (double)RAND_MAX;
    r *= (RMAX - 1);
    r += 1;

    return (size_t)r;
}

// Check if a number is prime
size_t isPrime(size_t n){
    size_t i;
    for (i = 2; i <= n/2; i++){
        if (n%i == 0) return 0;
    }
    return 1;
}

// Find an empty position in array
size_t findEmpty(size_t *shared, size_t size){
    size_t i;
    for (i = 0; i < size; i++){
        if (shared[i] == EMPTY){
            return i;
        }
    }
    return -1;
}

// Find a filled position in an array
size_t findNotEmpty(size_t *shared, size_t size){
    size_t i;
    for (i = 0; i < size; i++){
        if (shared[i] != EMPTY){
            return i;
        }
    }
    return -1;
}

// Producer thread function
void *produce(void *arg){
    // Get thread data pointer
    thread_data *t_data = (thread_data *)arg;

    // Unpack variables
    size_t iterations = t_data->iterations;
    size_t *shared = t_data->shared;
    size_t size = t_data->size;

    // Generate random numbers and put them on shared memory
    size_t i, r, pos;
    for (i = 0; i < iterations; i++){
        r = genrandom();
        // Wait for space to be available to write to
        sem_wait(&semWrite);
        // Lock access so only one write happens at a time
        pthread_mutex_lock(&mutexWrite);
        pos = findEmpty(shared, size);
        shared[pos] = r;
        // Unlock access
        pthread_mutex_unlock(&mutexWrite);
        // Signal read access
        sem_post(&semRead);
    }

    pthread_exit(NULL);
}

// Consumer thread function
void *consume(void *arg){
    // Get thread data pointer
    thread_data *t_data = (thread_data *)arg;

    // Unpack variables
    size_t iterations = t_data->iterations;
    size_t *shared = t_data->shared;
    size_t size = t_data->size;

    // Read numbers from memory and print whether they are prime or not
    size_t i, val, pos, isp;
    for (i = 0; i < iterations; i++){
        // Wait for something to be available to read from
        sem_wait(&semRead);
        // Lock access so only one read happens at a time
        pthread_mutex_lock(&mutexRead);
        pos = findNotEmpty(shared, size);
        val = shared[pos];
        shared[pos] = 0;
        // Unlock access
        pthread_mutex_unlock(&mutexRead);
        // Signal write access
        sem_post(&semWrite);
        isp = isPrime(val);
        if (isp){
            //printf("Number %zu is prime!\n", val);
        }
        else{
            //printf("Number %zu is not prime!\n", val);
        }
    }

    pthread_exit(NULL);
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

    // Initialize mutexes
    pthread_mutex_init(&mutexRead, NULL);
    pthread_mutex_init(&mutexWrite, NULL);

    // Initialize semaphores
    sem_init(&semRead, 0, 0);
    sem_init(&semWrite, 0, size);

    // Start variable that will act as shared memory - in heap
    size_t *shared;
    shared = (size_t *)malloc(sizeof(size_t) * size);
    size_t i;
    for (i = 0; i < size; i++){
        shared[i] = 0;
    }

    // Compute individual producer load and remainder
    size_t p_load, p_rest;
    p_load = RANDOMS / producers;
    p_rest = RANDOMS - (p_load * producers);

    // Compute individual consumer load and remainder
    size_t c_load, c_rest;
    c_load = RANDOMS / consumers;
    c_rest = RANDOMS - (c_load * consumers);

    // Generate producer threads
    size_t t, rc;
    for (t = 0; t < producers; t++){
        // Initialize thread data for this thread
        thread_data_array[t].thread_id = t; // Id is current iteration
        if (t == (producers - 1)){
            // Last thread should work slightly more to cover for uneven division
            thread_data_array[t].iterations = p_load + p_rest;
        }
        else {
            thread_data_array[t].iterations = p_load;
        }
        thread_data_array[t].shared = shared; // Set shared memory pointer
        thread_data_array[t].size = size; // Set shared memory size
        rc = pthread_create(&threads[t], &attr, produce, (void *) &thread_data_array[t]);
        if (rc){
            perror("Could not create thread");
            exit(-1);
        }
    }

    // Generate consumer threads
    size_t s;
    for (t = 0; t < consumers; t++){
        s = producers + t;
        // Initialize thread data for this thread
        thread_data_array[s].thread_id = s; // Id is current iteration
        if (t == (consumers - 1)){
            // Last thread should work slightly more to cover for uneven division
            thread_data_array[s].iterations = c_load + c_rest;
        }
        else {
            thread_data_array[s].iterations = c_load;
        }
        thread_data_array[s].shared = shared; // Set shared memory pointer
        thread_data_array[s].size = size; // Set shared memory size
        rc = pthread_create(&threads[s], &attr, consume, (void *) &thread_data_array[s]);
        if (rc){
            perror("Could not create thread");
            exit(-1);
        }
    }

    void *status;
    // Wait for all threads to join back
    for (t = 0; t < n_threads; t++){
        rc = pthread_join(threads[t], &status);
        if (rc){
            perror("Thread returned an error");
            exit(-1);
        }
    }

    // Free memory
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutexRead);
    pthread_mutex_destroy(&mutexWrite);
    sem_destroy(&semRead);
    sem_destroy(&semWrite);
    free(thread_data_array);
    free(shared);

    return 0;
}
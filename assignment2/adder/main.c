#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutexsum; // Globally visible mutex

// Information to be passed along to each thread
typedef struct{
    size_t thread_id; // Thread ID
    size_t iterations; // How many elements to iterate on data
    signed char *data; // Data to sum
    signed long long int *sum; // Where to sum elements to
} thread_data;

// Code to be executed by each thread - sums elements in data
void *threadSum(void *arg){
    // Get thread data pointer
    thread_data *t_data = (thread_data *)arg;

    // Unpack variables
    size_t iterations = t_data->iterations;
    signed char *data = t_data->data;
    signed long long int sum = 0;
    signed long long int *sum_dest = t_data->sum;

    // Iterate data, adding to sum
    size_t i = 0;
    while (i < iterations){
        sum += data[i++];
    }

    //Finished adding, put result on heap variable
    while(pthread_mutex_trylock(&mutexsum));
    *sum_dest += sum;
    pthread_mutex_unlock(&mutexsum);

    pthread_exit(NULL);
}

// Generate n random numbers between -100 and 100
signed char *generate_numbers(size_t n){

    // Call memory for n numbers -127 to 127
    signed char *numbers = (signed char *)malloc(sizeof(signed char) * n);
    if (!numbers){
        perror("Not enough memory");
        exit(-1);
    }

    size_t i;
    double random;
    for (i = 0; i < n; i++){
        // Generate random number from -100 to 100
        random = (double)rand() / (double)RAND_MAX;
        random *= 200;
        random -= 100;
        numbers[i] = (signed char)random;
    }

    return numbers;

}

int main(int argc, char const *argv[]){

    srand(time(NULL));

    size_t n_numbers; // Number can exceed 2^32 - 1
    size_t n_threads;
    char spinlock = 1; // Using char for saving memory space - every byte counts!

    // Parse arguments
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("Usage: ./adder <N> <K> <busy wait flag>\n"
               "       N: Number of numbers to generate\n"
               "       K: Number of threads to use\n"
               "       To avoid busy wait, use --no_spinlock\n");
        exit(0);
    }

    else if (argc > 3 || argc < 2){
        printf("Arguments specified wrongly. Use --help for usage.\n");
        exit(-1);
    }

    // Get number of numbers to generate
    n_numbers = atoll(argv[1]);

    // Get number of threads to use
    n_threads = atoi(argv[2]);

    // Generate N random numbers between -100 and 100
    signed char *numbers = generate_numbers(n_numbers);

    // Configure pthread - number of threads
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

    // Initialize mutex
    pthread_mutex_init(&mutexsum, NULL);

    // Start variable that will act as a sum counter
    signed long long int *sum;
    sum = (signed long long int *)malloc(sizeof(signed long long int));
    *sum = 0;

    // Compute individual thread load and remainder in case of uneven division
    size_t load = n_numbers / n_threads;
    size_t rest = n_numbers - (load * n_threads);

    // Create threads and assign work for them
    size_t t, rc;
    for (t = 0; t < n_threads; t++){
        // Initialize thread data for this thread
        thread_data_array[t].thread_id = t; // Id is current iteration
        if (t == (n_threads - 1)){
            // Last thread should work slightly more to cover for uneven division
            thread_data_array[t].iterations = load + rest;
        }
        else {
            thread_data_array[t].iterations = load;
        }
        // Pass a reference to the first number the thread should compute
        thread_data_array[t].data = &numbers[t * load];
        thread_data_array[t].sum = sum;
        rc = pthread_create(&threads[t], &attr, threadSum, (void *) &thread_data_array[t]);
        if (rc){
            perror("Could not create thread");
            exit(-1);
        }
    }

    // Free thread attribute - no more threads to make
    pthread_attr_destroy(&attr);

    void *status;
    // Wait for all threads to join back
    for (t = 0; t < n_threads; t++){
        rc = pthread_join(threads[t], &status);
        if (rc){
            perror("Thread returned an error");
            exit(-1);
        }
    }

    printf("Finished runnning. Final result: %lli.\n", *sum);

    // Free memory
    pthread_mutex_destroy(&mutexsum);
    free(numbers);
    free(thread_data_array);
    free(sum);

    return 0;
}

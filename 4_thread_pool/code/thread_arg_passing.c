#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS     5

struct thread_data {
    int thread_id;
    int sum;
    char *message;
};


void *PrintHello(void *thread_arg) {
    struct thread_data *my_data;

    my_data = (struct thread_data *) thread_arg;
    int task_id = my_data->thread_id;
    int sum = my_data->sum;
    char *hello_message = my_data->message;
    printf("Thread %d: %s Sum=%d\n", task_id, hello_message, sum);
    pthread_exit(NULL);
}

int main(void) {
    int rc, sum;

    sum = 0;
    pthread_t thread[NUM_THREADS];

    struct thread_data thread_data_array[NUM_THREADS];
    char messages[NUM_THREADS][100];

    for(int i = 0; i < NUM_THREADS; i++) {
        sum = sum + i;

        struct thread_data *data = &thread_data_array[i];
        data->thread_id = i;
        data->sum = sum;
        sprintf(messages[i], "This is hello message from thread: %d", i);
        data->message = messages[i]; 

        printf("Createing thread %d\n", i);
        rc = pthread_create(&thread[i], NULL, PrintHello, (void *)data);
    
        if (rc) {
            printf("Failed return code from pthread_create(): %d\n", rc);
            perror("Perror");
            exit(-1);
        }
    }
    pthread_exit(NULL);
}

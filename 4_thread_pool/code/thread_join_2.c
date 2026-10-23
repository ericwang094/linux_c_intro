#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS     5

struct thread_data {
    int thread_id;
    int thread_sum;
};

void *BusyWork(void *thread_arg) {
    struct thread_data *t_d = (struct thread_data *)thread_arg; 

    int thread_id = t_d->thread_id;

    printf("Inside of thread id: %d\n", thread_id);
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }
    t_d->thread_sum = sum;

    // pthread_exit((void *) t_d);
    pthread_exit(NULL);
}

int main(void) {

    pthread_t threads[NUM_THREADS];
    pthread_attr_t thread_attr;

    struct thread_data t_data[NUM_THREADS];

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_data *t_d = &t_data[i];
        t_d->thread_id = i;

        printf("Creating pthread id %d: \n", i);
        int rc = pthread_create(&threads[i], &thread_attr, BusyWork, (void *) t_d);

        if (rc) {
            printf("Failed, pthread_create() return code %d\n", rc);
            exit(-1);
        }
    }

    pthread_attr_destroy(&thread_attr);

    for (int i = 0; i < NUM_THREADS; i++) {
        // struct thread_data *ret;
        // int rc = pthread_join(threads[i], (void **)&ret);
        int rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("Pthread_join return code: %d\n", rc);
            exit(-1);
        }

        printf("In main, we get data from thread, thread id: %d, and sum: %d\n", t_data[i].thread_id, t_data[i].thread_sum); 
        // printf("In main, we get data from thread, thread id: %d, and sum: %d\n", ret->thread_id, ret->thread_sum); 
    }


    pthread_exit(NULL);
    return 0;
}

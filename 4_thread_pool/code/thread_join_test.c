#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS     5

struct thread_data {
    int thread_id;
    int thread_sum;
};

void *BusyWork(void * thread_arg) {
    struct thread_data *t_data = (struct thread_data *) thread_arg;

    printf("Inside of thread: %d", t_data->thread_id);
    t_data->thread_sum = t_data->thread_id;

    pthread_exit(NULL);
}

int main() {

    pthread_t *threads[NUM_THREADS];
    struct thread_data *thread_data[NUM_THREADS]; // variable name same as struct name, ok?
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < NUM_THREADS; i++) {

        struct thread_data *t_data = thread_data[i];
        t_data->thread_id = i;  // what is the value of t_data->thread_sum if not initialized?

        int rc = pthread_create(threads[i], &thread_attr, BusyWork, (void **)&t_data);
        if (rc) {
            printf("Error\n");
            exit(-1);
        }
    }

    pthread_attr_destroy(&thread_attr);


    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(*threads[i], NULL);
        struct thread_data *t_data = thread_data[i];

        printf("In main, thread id: %d\n has sum: %d\n", t_data->thread_id, t_data->thread_sum);
    }


    pthread_exit(NULL);

}

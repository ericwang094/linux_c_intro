#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS     4

// void *BusyWork(void *t) {
//     int i;
//     long tid;
//     double result = 0.0;
//     tid = (long)t;
//     printf("THread %ld starting...\n", tid);
//     for (i = 0; i < 1000000; i++) {
//         result = result + sin(i) * tan(i);
//     }
//
//     printf("Thread %ld done. Result = %e\n", tid, result);
//     pthread_exit((void *) t);
// }
//
// int main(int argc, char *argv[]) {
//     pthread_t thread[NUM_THREADS];
//     pthread_attr_t attr;
//     int rc;
//     long t;
//     void *status;
//
//     pthread_attr_init(&attr);
//     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
//
//     for (t = 0; t < NUM_THREADS; t++) {
//         printf("Main: creating thread %ld\n", t);
//         rc = pthread_create(&thread[t], &attr, BusyWork, (void *)t);
//         if (rc) {
//             printf("Error; return code from pthread_create() is %d\n", rc);
//             exit(-1);
//         }
//     }
//
//     pthread_attr_destroy(&attr); // NOTE: this attr is only used when thread is created. So after creation thread, this needs to be destroyed.
//     for (t = 0; t < NUM_THREADS; t++) {
//         rc = pthread_join(thread[t], &status);
//         if (rc) {
//             printf("Error; return code from pthread_join() is %d\n", rc);
//             exit(-1);
//         }
//
//         printf("Main: completed join with thread %ld having a status of %ld\n", t, (long)status);
//     }
//
//     printf("Main: program completed. Existing.\n");
//     pthread_exit(NULL);
// }

struct thread_data {
    int thread_id;
};

struct thread_result {
    double thread_sum;
};

void *BusyWork(void *thread_arg) {
    struct thread_data *data; 
    data = (struct thread_data *)thread_arg;

    double thread_sum = 0;

    int thread_id = data->thread_id;
    for (int i = 0; i < 1000000; i++) {
        thread_sum = thread_sum + sin(i) * tan(i);
    }

    printf("Inside thread: %d doing sum: %f\n", thread_id, thread_sum);
    // struct thread_result *thread_result = {0}; // TODO: why this is not right
    struct thread_result *t_result = malloc(sizeof(*t_result));

    t_result->thread_sum = thread_sum;

    pthread_exit((void *) t_result);
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct thread_data t_data[NUM_THREADS];

    pthread_attr_t thread_attr;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_data *t_d = &t_data[i];
        t_d->thread_id= i;

        printf("Creating threads: %d\n", i);
        int rc = pthread_create(&threads[i], &thread_attr, BusyWork, (void *)t_d);
        if (rc) {
            printf("Error code return from pthread_create() with: %d", rc);
            exit(-1);
        }
    }

    pthread_attr_destroy(&thread_attr);

    struct thread_result *t_result[NUM_THREADS] = {0};

    for (int i = 0; i < NUM_THREADS; i++) {
        int rc = pthread_join(threads[i], (void **)&t_result[i]);
        if (rc) {
            printf("Pthread_join failed with error code: %d\n", rc);
            exit(-1);
        }
        printf("In main, we get thread: %d, join result: %f\n",i, t_result[i]->thread_sum);

        free(t_result[i]);
    }

    pthread_exit(NULL);
}

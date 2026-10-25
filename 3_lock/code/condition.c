#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS     3
#define TCOUNT          10
#define COUNT_LIMIT     12

int count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threadhold_cv;


void *inc_count(void *t) {
    int i;
    long my_id = (long)t;

    for (i = 0; i < TCOUNT; i++) {
        pthread_mutex_lock(&count_mutex);
        count++;

        /* Check the value of count and signal waiting thread when condition
        * id reached. Note that this occurs while mutex is locked.
        */

        if (count == COUNT_LIMIT) {
            printf("inc_count: thread %ld, count = %d -- threadhold reached.", my_id, count);
            pthread_cond_signal(&count_threadhold_cv);
            printf("Just sent signal.\n");
        }

        printf("inc_count(): thread %ld, count = %d -- unlocking mutex\n", my_id, count);
        pthread_mutex_unlock(&count_mutex);

        // do some work so threads can alternate on mutex lock
        sleep(1);
    }
    pthread_exit(NULL);
}


void *watch_count(void *t) {
    long my_id = (long)t;

    printf("starting watch_count(): thread %ld\n", my_id);

    /*
    * Lock mutex and wati for sgnial. Note that the pthread_count_wait routine
    * will automatically and atomically unlock mutex while it waits.
    * Also, note that if COUNT_LIMIT is reached before this routine is run by
    * the waiting thread, the loop will be skipped to prevent pthread_count_wait from never returning.
    */
    pthread_mutex_lock(&count_mutex);
    while (count < COUNT_LIMIT) {
        printf("watch_count(): thread %ld Count = %d. Going to wait..\n", my_id, count);
        pthread_cond_wait(&count_threadhold_cv, &count_mutex);
        printf("watch_count(): thread %ld Condition signal received. Count = %d\n", my_id, count);
    }

    printf("watch_count(): thread %ld updating the value of count...\n", my_id);
    count += 125;
    printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
    printf("watch_count(): thread %ld unlocking mutex.\n", my_id);
    pthread_mutex_unlock(&count_mutex);
    pthread_exit(NULL);
}


int main(void) {
    int i;
    long t1 = 1,
         t2 = 2,
         t3 = 3;
    pthread_t threads[3];
    pthread_attr_t attr;

    // initialze mutex and condition variable objects
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threadhold_cv, NULL);


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, watch_count, (void *)t1);
    pthread_create(&threads[1], &attr, inc_count, (void *)t2);
    pthread_create(&threads[2], &attr, inc_count, (void *)t3);

    // wait for all threads to complete
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Main(): waited and joined with %d threads. Final value of count = %d. Done.\n", NUM_THREADS, count);


    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threadhold_cv);
    pthread_exit(NULL);
}

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS      5
#define COUNT_LIMIT      20

int count = 0;
pthread_mutex_t mutex_t;
pthread_cond_t cond_t;


void * inc_count(void * thread_arg) {
    long thread_id;
    thread_id = (long)thread_arg;

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex_t);
        count++;
        printf("In inc_count thread_id: %ld, current count: %d\n", thread_id, count);
        if (count == COUNT_LIMIT) {
            pthread_cond_signal(&cond_t);
        }
        pthread_mutex_unlock(&mutex_t);
    }

    pthread_exit(NULL);
}


void * watch_count(void * thread_arg) {
    long thread_id;
    thread_id = (long)thread_arg;
    pthread_mutex_lock(&mutex_t);

    while (count < COUNT_LIMIT) {
        printf("In watch_count() in thread id %ld, current count: %d, waiting...\n", thread_id, count);
        pthread_cond_wait(&cond_t, &mutex_t);
        printf("In watch_count(), signal received...\n");
    }

    count += 100;
    pthread_mutex_unlock(&mutex_t);

    pthread_exit(NULL);
}


int main(void) {

    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&mutex_t, NULL);
    pthread_cond_init(&cond_t, NULL);

    for (long i = 0; i < 5; i++) {
        if (i == 0) {
            int rc = pthread_create(&threads[i], NULL, watch_count, (void *)i);
            if (rc) {
                printf("Error creating thread_id: %ld\n", i);
                exit(-1);
            }
        } else {
            int rc = pthread_create(&threads[i], NULL, inc_count, (void *)i);
            if (rc) {
                printf("Error creating thread_id: %ld\n", i);
                exit(-1);
            }
        }
    }

    for(int i = 0; i <NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_t);
    pthread_cond_destroy(&cond_t);
    pthread_exit(NULL);

}

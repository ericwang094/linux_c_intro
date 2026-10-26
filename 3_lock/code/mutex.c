#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double *a;
    double *b;
    double sum;
    int veclen;
} DOTDATA;

#define NUM_THREADS     4
#define VECLEN          100


DOTDATA dotstr;
pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutex_sum;

void *dotprod(void *arg) {
    int i, start, end ,len;
    long offset;
    double my_sum, *x, *y;

    offset = (long)arg;

    len = dotstr.veclen;
    start = offset * len;
    end = start + len;

    x = dotstr.a;
    y = dotstr.b;

    my_sum = 0;
    for (i = start; i < end; i++) {
        my_sum += (x[i] * y[i]);
    }

    pthread_mutex_lock(&mutex_sum);
    dotstr.sum += my_sum;
    pthread_mutex_unlock(&mutex_sum);

    pthread_exit((void *) 0);
}


int main(void) {
    long i;
    double *a, *b;
    void *status;
    pthread_attr_t attr;

    a = (double *)malloc(NUM_THREADS * VECLEN * sizeof(double));
    b = (double *)malloc(NUM_THREADS * VECLEN * sizeof(double));

    for (i = 0; i < VECLEN * NUM_THREADS; i++) {
        a[i] = 1.0;
        b[i] = a[i];
    }

    dotstr.veclen = VECLEN;
    dotstr.a = a;
    dotstr.b = b;
    dotstr.sum = 0;

    pthread_mutex_init(&mutex_sum, NULL);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&callThd[i], &attr, dotprod, (void *)i);
    }

    pthread_attr_destroy(&attr);

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(callThd[i], &status);
    }

    printf("Sum = %f\n", dotstr.sum);
    free(a);
    free(b);
    pthread_mutex_destroy(&mutex_sum);
    pthread_exit(NULL);
}

// #include <stdio.h>
// #include <pthread.h>
// #include <stdlib.h>
//
// #define NUM_THREAD      5
//
// void *PrintHello(void *thread_id) {
//     long tid;
//     tid = (long)thread_id;
//     printf("hello world! it's me thread #%ld!\n", tid);
//     pthread_exit(NULL);
// }
//
// int main() {
//     pthread_t threads[NUM_THREAD];
//     int rc;
//     long t;
//     for (t = 0; t < NUM_THREAD; t++) {
//         printf("In main: creating thread %ld\n", t);
//         rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
//         if (rc) {
//             printf("Error; return code from pthread_created() is %d\n", rc);
//             exit(-1);
//         }
//     }
//
//     pthread_exit(NULL);
//     return 0;
// }
//

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREAD      5

void* thread_callback(void *arg) {
    long thread_id = (long) arg;
    printf("Hello world, this is %ld\n", thread_id);
    pthread_exit(NULL);
}

int main(void) {

    pthread_t threads[NUM_THREAD];

    int rc;
    for (long i = 0; i < NUM_THREAD; i++) {
        printf("In main, creating thread: %ld\n", i);
        rc = pthread_create(&threads[i], NULL, thread_callback, (void *)i);
        if (rc) {
            printf("Failed creating thread %d\n", rc);
            perror("Thread error");
            exit(-1);
        }
    }

    // iwthout this, the main might finish before the thread finsih therefore, killing all threads
    // // calling this in main will block the main untill the thread are done.
    pthread_exit(NULL);
}

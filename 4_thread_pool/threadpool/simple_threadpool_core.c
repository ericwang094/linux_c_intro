#include "simple_threadpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct tpool_work {
    thread_func_t   func;
    void            *arg;
    struct tpool_work  *next;
};
typedef struct tpool_work tpool_work_t;


struct tpool {
    tpool_work_t  *work_first;
    tpool_work_t  *work_last;
    pthread_mutex_t work_mutex;
    pthread_cond_t  work_cond;      // signals the threads that there is work to be processed
    pthread_cond_t  working_cond;   // there are no threads processing
    size_t      working_cnt;
    size_t      thread_cnt;     // why size_t  not int
    bool       stop;    // is bool recognised by c? shouldn't it be __Bool?
};


static tpool_work_t *tpool_work_create(thread_func_t func, void *arg) {
    // tpool_work_t construct function
    tpool_work_t *work;
    if (func == NULL) {
        return NULL;
    }

    work = malloc(sizeof(*work));
}

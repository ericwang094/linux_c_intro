#include "simple_threadpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct tpool_work {
    thread_func_t      func;
    void              *arg;
    struct tpool_work *next;
};
typedef struct tpool_work tpool_work_t;

struct tpool {
    tpool_work_t   *work_first;
    tpool_work_t   *work_last;
    pthread_mutex_t work_mutex;
    pthread_cond_t  work_cond;    // signals the threads that there is work to be processed
    pthread_cond_t  working_cond; // there are no threads processing
    size_t          working_cnt;
    size_t          thread_cnt; // why size_t  not int
    bool            stop;       // is bool recognised by c? shouldn't it be __Bool?
};

static tpool_work_t *tpool_work_create(thread_func_t func, void *arg) {
    // tpool_work_t construct function
    tpool_work_t *work;
    if (func == NULL) {
        return NULL;
    }

    work       = malloc(sizeof(*work));
    work->func = func;
    work->arg  = arg;
    work->next = NULL;
    return work;
}

static void tpool_work_destroy(tpool_work_t *work) {
    if (work == NULL) {
        return;
    }

    free(work);
}

static tpool_work_t *tpool_work_get(tpool_t *tm) {
    // pulling work/job from the queue
    tpool_work_t *work;

    if (tm == NULL) {
        return NULL;
    }

    // get the first element from the queue
    work = tm->work_first;
    if (work == NULL) {
        return NULL;
    }

    // if the next element is null, meaning this is the last job in the queue
    // update the tpool_t, set the queue to be null
    if (work->next == NULL) {
        tm->work_first = NULL;
        tm->work_last  = NULL;
    } else {
        // update the queue from the pool, make the next work/job to be the next available work
        tm->work_first = work->next;
    }

    return work;
}

static void *tpool_worker(void *arg) {
    tpool_t      *tm = arg;
    tpool_work_t *work;

    while (1) {
        pthread_mutex_lock(&(tm->work_mutex));

        while (tm->work_first == NULL &&
               !tm->stop) { // TODO: confirm again why need to use while loop
            // no work available to pickup, wait
            pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));
        }

        if (tm->stop) {
            break;
        }

        work = tpool_work_get(tm);
        tm->working_cnt++;
        pthread_mutex_unlock(&(tm->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tm->work_mutex));
        tm->working_cnt--;
        if (!tm->stop && tm->working_cnt == 0 &&
            tm->work_first == NULL) { // TODO: why tm->work_first is NULL we need to send signal?
            pthread_cond_signal(&(tm->working_cond));
        }

        pthread_mutex_unlock(&(tm->work_mutex));
    }

    tm->thread_cnt--; // TODO: Why no lock here? wouldn't all threads try to decrease this value, if
                      // multi-threads try update this value, we might end up the tm->thread_cnt
                      // greater than 0 even if the actual thread count is 0.
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    return NULL;
}

tpool_t *tpool_create(size_t num) {
    tpool_t  *tm;
    pthread_t thread;
    size_t    i;

    // if (num == 0) {
    //     num = 2;
    // }

    // Unlike malloc(), which leaves allocated memory uninitialized (containing garbage values),
    // calloc() automatically initializes every allocated byte to 0
    tm             = calloc(1, sizeof(*tm)); // TODO: when use malloc and when use calloc?
    tm->thread_cnt = num;

    pthread_mutex_init(&(tm->work_mutex), NULL);
    pthread_cond_init(&(tm->work_cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);

    tm->work_first = NULL;
    tm->work_last  = NULL;

    for (i = 0; i < num; i++) {
        pthread_create(&thread, NULL, tpool_worker, tm);
        pthread_detach(thread); // when to use detach?
    }

    return tm;
}

void tpool_destroy(tpool_t *tm) {
    tpool_work_t *work;
    tpool_work_t *work2;

    if (tm == NULL) {
        return;
    }

    pthread_mutex_lock(&(tm->work_mutex));
    work = tm->work_first;
    while (work != NULL) {
        work2 = work;
        tpool_work_destroy(work);
        work = work2;
    }

    tm->work_first = NULL;
    tm->stop       = true;
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    // some threads may have already been running and are currently processing,
    // so we should wait for them to finish
    tpool_wait(tm);

    pthread_mutex_unlock(&(tm->work_mutex));
    pthread_cond_destroy(&(tm->work_cond));
    pthread_cond_destroy(&(tm->working_cond));

    free(tm);
}

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg) {
    tpool_work_t *work;
    if (tm == NULL) {
        return false;
    }

    work = tpool_work_create(func, arg);
    if (work == NULL) {
        return false;
    }

    pthread_mutex_lock(&(tm->work_mutex));
    if (tm->work_first == NULL) {
        tm->work_first = work;
        tm->work_last  = tm->work_first;
    } else {
        tm->work_last->next = work;
        tm->work_last       = work;
    }

    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    return true;
}

void tpool_wait(tpool_t *tm) {
    if (tm == NULL) {
        return;
    }

    pthread_mutex_lock(&(tm->work_mutex));
    while (1) {
        if (tm->work_first != NULL || (!tm->stop && tm->working_cnt != 0) ||
            (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}

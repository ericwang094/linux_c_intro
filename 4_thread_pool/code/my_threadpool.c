#include <pthread.h>
#include <stdlib.h>
#include <pthread.h>

struct nTask {
    void (*task_function)(void *arg);
    void *user_data;

    struct nTask *prev;
    struct nTask *next;
};

struct nWorker {
    pthread_t threadid;
    int terminate;
    struct nManager *manager;

    struct nWorker *prev;
    struct nWorker *next;
};

typedef struct nManager {
    struct nTask *tasks;
    struct nWorker *workers;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ThreadPool;



static void *nThreadPoolCalback(void *arg) {
    struct nWorker *worker = (struct nWorker*) arg;

    while (1) {
        pthread_mutex_lock(&worker->manager->mutex);
        while (worker->manager->tasks == NULL) {
            if (worker->terminate) break;
            pthread_cond_wait(&worker->manager->cond, &worker->manager->mutex);
        }
        if (worker->terminate) {
            pthread_mutex_unlock(&worker->manager->mutex);
            break;
        }
        struct nTask *task = worker->manager->tasks;
        LIST_REMOVE(task, worker->manager->tasks);

        pthread_mutex_unlock(&worker->manager->mutex);

        task->task_func(task->user_data);
    }

    free(worker);
}


int nThreadPoolCreate(ThreadPool *pool, int numWorkers) {

    if (pool == NULL) return -1;
    if (numWorkers < 1) numWorkers = 1;

    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    memcpy(&pool->cond, &blank_cond, sizeof(pthread_count_t));

    pthread_mutex_init(&pool->mutex, NULL);

    int i = 0;
    for (i = 0; i < numWorkers; i++) {
        struct nWorker *worker = (struct nWorker*)malloc(sizeof( struct nWorker));
        if (worker == NULL) {
            perror("Malloc");
            return -2;
        }
        memset(worker, 0, sizeof(struct nWorker));
        worker->manager = pool;

        int ret = pthread_create(worker->manager, NULL, nThreadPoolCallback, worker);
        if (ret) {
            perror("pthread_create");
            free(worker);
            return -3;
        }

        LIST_INSERT(worker, pool->workers);
    }


    return 0;
}


int nThreadPoolDestory(ThreadPool *pool, int nWorker) {

    struct nWorker *worker = NULL;

    for (worker = pool->workers; worker != NULL; worker = worker->next) {
        worker->terminate = 1;
    }

    pthread_mutex_lock(&pool->mutex);
    pthread_cond_broadcast(&pool->cond);  // awake all
    pthread_mutex_unlock(&pool->mutex);

    pool->workers = NULL;
    pool->tasks = NULL;


    return 0;
}

int nThreadPoolPushTask(ThreadPool *pool, struct nTask *task) {
    
    pthread_mutex_lock(&pool->mutex);
    LIST_INSERT(task, pool->tasks);
    pthread_cond_signal(&pool->cond); // awake single one
    pthread_mutex_unlock(&pool->mutex);
}


#if 1

#define THREADPOOL_INIT_COUNT   20
#define TASK_INIT_SIZE           1000

void task_entry(struct nTask *task) { //type 

	//struct nTask *task = (struct nTask*)task;
	int idx = *(int *)task->user_data;

	printf("idx: %d\n", idx);

	free(task->user_data);
	free(task);
}

int main(void) {
    ThreadPool pool = {0};

    nThraedPoolCreate(&pool, THREADPOOL_INIT_COUNT);

    int i = 0;
    for (i = 0; i < TASK_INIT_SIZE; i++) {
        struct nTask *task(struct nTasks*) malloc(sizeof(struct nTask)); 
        if (task == NULL) {
            perror("malloc");
            exit(1);
        }

    memset(task, 0, sizeof(struct nTask));

    task->task_func = task_entry;
    task->user_data = malloc(size(int));
    *(int*) task->user_data = i;

    nThreadPoolPushTask(&pool, task);
    }
}

// 了解下CAS compare and swap
# include <stdio.h>
# include <pthread.h>
# include <unistd.h>

# define THREAD_COUNT   10

pthread_mutex_t mutex;
pthread_spinlock_t spin_lock;

int inc(int *value, int add) {
    int old;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile(
        "lock; xaddl %2, %1;"
        : "=a" (old), "+m" (*value)
        : "a"(add)
        : "cc", "memory"
    );
#elif defined(__aarch64__)
    int tmp;
    unsigned int status;

    // ldaxr/stlxr: 独占加载/存储，stlxr 失败(status != 0)说明被别的核改过，重试
    __asm__ volatile(
        "1: ldaxr %w0, %3\n"
        "   add   %w1, %w0, %w4\n"
        "   stlxr %w2, %w1, %3\n"
        "   cbnz  %w2, 1b\n"
        : "=&r" (old), "=&r" (tmp), "=&r" (status), "+Q" (*value)
        : "r" (add)
        : "memory"
    );
#else
    old = __sync_fetch_and_add(value, add);
#endif

    return old;
}

void *thread_callback(void *arg) {
    int *pcount = (int *)arg;

    int i = 0;

    while (i++ < 100000) {

#if 0
        (*pcount)++;
#elif 0
        pthread_mutex_lock(&mutex);
        (*pcount)++;  //
        pthread_mutex_unlock(&mutex);
#elif 0
        pthread_spin_lock(&spin_lock);
        (*pcount)++;  //
        pthread_spin_unlock(&spin_lock);
#else
        inc(pcount, 1);
#endif
        usleep(1);
    }

    return NULL;
}

int main() {
    pthread_t threadid[THREAD_COUNT] = {0};
    pthread_mutex_init(&mutex, NULL);
    pthread_spin_init(&spin_lock, PTHREAD_PROCESS_SHARED);

    int i = 0;
    int count = 0;

    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threadid[i], NULL, thread_callback, &count);
    }

    while(1) {
        printf("count: %d\n", count);
        sleep(1);
    }
}

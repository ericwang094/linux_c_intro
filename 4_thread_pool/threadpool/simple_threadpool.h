#ifndef __TPOOL_H__  // TODO: what is this
#define __TPOOL_H__


#include <stdbool.h>
#include <stddef.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void *arg);  // TODO: why declare typedef? what is the purpose


tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t fun, void *arg);
void tpool_wait(tpool_t *tm);

#endif /* __TPOOL_H__ */

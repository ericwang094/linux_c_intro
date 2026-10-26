1. simple_threadpool.h:1：#ifndef __TPOOL_H__ 是什么

这是 include guard（头文件保护），防止同一个头文件在一个 .c 里被展开两次。

#ifndef __TPOOL_H__   // 如果还没定义过这个宏
#define __TPOOL_H__   // 先定义它
...                   // 头文件内容
#endif                // 第二次 include 时宏已存在，整段被跳过
比如 a.h 和 b.h 都 #include "simple_threadpool.h"，而 main.c 同时 include 了 a.h 和 b.h。没有保护的话，typedef struct tpool tpool_t; 这类定义就会重复出现。

还有一个小问题：C 标准规定以两个下划线开头（或者下划线加大写字母开头）的名字是保留给编译器和标准库用的。更规范的写法是 SIMPLE_THREADPOOL_H。也可以直接写 #pragma once，它不在标准里，。

2. simple_threadpool.h:11：为什么要 typedef 函数指针

thread_func_t 是"参数为 void *、没有返回值的函数"的指针类型。  ，所以让用户传一个函数和一个 void * 参数进来，worker线程拿到之后调用 work->func(work->arg)。void * 可以指向任何数据，这是 C 里实现"泛型回调"的常见写法。

用 typedef 是为了好读。不用的话，每个用到它的地方都要写成：
bool tpool_add_work(tpool_t *tm, void (*func)(void *arg), void

struct tpool_work {
    void (*func)(void *arg);
    ...
};
函数指针的语法很难读，用了 typedef 之后写成 thread_func_t func三个参数 void *(*)(void *) 也是同一个思路。

3. simple_threadpool_core.c:20：为什么用 size_t 而不是 int

- size_t 是无符号的，专门用来表示"大小"和"个数"，不会是负数。
- 它和 sizeof、malloc、calloc、数组下标这些用的是同一个类型。calloc(num_items, sizeof(*vals)) 的参数就是 size_t，混用 int
  会引入有符号和无符号之间的转换。
- 它的宽度跟平台走，64 位系统上是 64 位。

注意：无符号数减到 0 以下会回绕成一个很大的数。所以 thread_cnt-- 这种地方必须保证逻辑正确，不能多减。

4. simple_threadpool_core.c:21：C 里有 bool 吗，不应该是 __Bool 吗

- C99 引入的关键字是 _Bool：一个下划线加大写 B，不是 __Bool。
- <stdbool.h> 里定义了三个宏：#define bool _Bool、#define true
- simple_threadpool.h:5 已经 #include <stdbool.h> 了，而 .c 又 include 了这个头文件，所以可以直接写 bool。
- 到了 C23，bool、true、false 已经直接是关键字，不需要再 inclu

5. simple_threadpool_core.c:81：为什么 pthread_cond_wait 外面

因为 pthread_cond_wait 返回的时候，条件不一定成立。有三种情况

1. 虚假唤醒（spurious wakeup）：POSIX 允许 pthread_cond_wait   。
2. 多个线程被同时唤醒，但只有一个能拿到任务：tpool_add_work 用的是 pthread_cond_broadcast，会把 4 个线程全叫醒，但队列里可能只有 1
   个任务。第一个拿到锁的线程取走任务后，其他 3 个线程醒来时队
3. 任务被别的线程抢先拿走：从被 signal 到重新拿到锁之间有一段时间，可能有另一个刚干完活的线程先拿到锁，把任务取走了。

所以醒来之后必须重新检查条件，不满足就继续睡。用 if 的话，第 90 行的 tpool_work_get 就可能拿到 NULL。这里第 94 行恰好判断了 work !=
NULL，所以不会崩溃，但 working_cnt 的计数逻辑会多绕一圈。

规则：pthread_cond_wait 永远放在 while 里。

6. simple_threadpool_core.c:102：为什么 work_first == NULL 时

发信号的对象是 tpool_wait()。它在等的是"所有任务都做完了"，看
tm->work_first != NULL || (!tm->stop && tm->working_cnt != 0)  // 有就继续等
"全部做完"要同时满足两个条件：
- working_cnt == 0：没有线程在执行任务
- work_first == NULL：队列里也没有排队的任务

如果只有 working_cnt == 0，但队列里还有任务，说明还没做完，马 ool_wait叫醒也没用，它检查一下条件又会继续睡。所以只有两个条件都满足时才 signal，避免无效唤醒。

!tm->stop 这个条件，是因为 destroy 阶段用的是另一套退出条件（thread_cnt），由第 112 行负责发信号。

7. simple_threadpool_core.c:109：为什么这里没加锁

其实是加了锁的，锁是从循环里带出来的：

while (1) {
    pthread_mutex_lock(&(tm->work_mutex));      // 78: 加锁
    while (... && !tm->stop) {
        pthread_cond_wait(...);                 // 83: 返回时
    }
    if (tm->stop) {
        break;                                  // 87: 带着锁跳出循环！
    }
    ...
}
tm->thread_cnt--;                               // 109: 这时还持有锁
pthread_cond_signal(&(tm->working_cond));
pthread_mutex_unlock(&(tm->work_mutex));        // 113: 这里才解锁
第 87 行 break 的时候没有解锁，所以第 109 行的 thread_cnt-- 是的竞争。第 113 行的 unlock 和第 78 行的 lock 是配对的。

这种"锁跨越 break"的写法容易让人误会，可以在第 87 行旁边加个注

8. simple_threadpool_core.c:129：什么时候用 malloc，什么时候用

┌──────────┬─────────────────────────────┬────────────────────────────┐
│          │        malloc(size)         │                   calloc(n, size)                    │
├──────────┼─────────────────────────────┼────────────────────────────┤
│ 内存内容 │ 未初始化（垃圾值）          │ 全部清零                                             │
├──────────┼─────────────────────────────┼────────────────────────────┤
│ 乘法溢出 │ 要自己算 n * size，可能溢出 │ 内部会检查 n * size 是否溢出                         │
├──────────┼─────────────────────────────┼────────────────────────────┤
│ 速度     │ 稍快                        │ 多一次清零（大块内存通常由系统直接给零页，差别不大） │
└──────────┴─────────────────────────────┴────────────────────────────┘

选择依据：后面会不会把每个字段都显式赋值？
- tpool_create 用 calloc 是对的。它依赖清零：stop 必须是 false，working_cnt 必须是 0，而这两个字段后面都没有手动赋值。
- tpool_work_create（第 31 行）用 malloc 也没问题，因为接下来
- 不确定的时候用 calloc，更安全。

另外，两处都没有检查返回值是否为 NULL，内存分配失败时会直接访问空指针。

9. simple_threadpool_core.c:141：什么时候用 pthread_detach

线程有两种状态：

- joinable（默认）：线程结束后资源不会马上释放，要等别的线程调用 pthread_join 来回收，还能拿到它的返回值。如果一直不 join，就会像僵尸进程一样泄漏资源。
- detached：线程结束后系统自动回收资源，但不能再 join，也拿不

什么时候用 detach：不关心线程什么时候结束、也不需要返回值的时
什么时候用 join：需要确认线程已经结束、需要它的返回值，或者在释放共享资源之前必须等线程退出的时候。

这个线程池选择了 detach，因为它用 thread_cnt 加条件变量自己实现了"等所有线程退出"（tpool_wait 在 stop 时等 thread_cnt == 0），所以不需要 join。而且第
120 行只用了一个 pthread_t thread 变量，每次循环都会被覆盖，本

更稳妥的写法是把 pthread_t 存成数组，tpool_destroy 时逐个 pthr程完全退出之后才 free(tm)。

---

顺便发现的 bug

1. tpool_destroy 里的链表释放写错了（第 156–161 行）：
work2 = work;              // 应该是 work2 = work->next;
tpool_work_destroy(work);
work = work2;              // work 还是那个已经被释放的节点
   如果调用 destroy 时队列里还有任务，就会重复 free 同一块内存estroy 之前调了 tpool_wait，队列已经空了，所以没有触发。
2. 第 172 行多了一个 pthread_mutex_unlock：这时锁已经在第 166 行和 tpool_wait 里释放了。解锁一个没被持有的 mutex 是未定义行为。这里应该换成 pthread_mutex_destroy(&(tm->work_mutex));，现在 mutex 从来
3. malloc 和 calloc 的返回值没有检查（见第 8 点）。

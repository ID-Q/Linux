# 线程池介绍

线程池允许一个线程可以多次复用，且每次复用的线程内部的消息处理可以不相同，将创建与销毁的开销省去而不必来一个请求开一个线程；简单来说就是有一堆已经创建好的线程（最大数目一定），初始时他们都处于空闲状态，当有新的任务进来，从线程池中取出一个空闲的线程处理任务，然后当任务处理完成之后，该线程被重新放回到线程池中，供其他的任务使用，当线程池中的线程都在处理任务时，就没有空闲线程供使用，此时，若有新的任务产生，只能等待线程池中有线程结束任务空闲才能执行。

# 应用场景

线程池是一种多线程处理形式，大多用于高并发服务器上，它能合理有效的利用高并发服务器上的线程资源；当我们的通信范围扩大到广域网或大型局域网通信中时，我们将面临大量消息频繁请求服务器；在这种情况下，创建与销毁线程都已经成为一种奢侈的开销，特别对于嵌入式服务器来说更应保证内存资源的合理利用。

# 线程池结构

## 线程池结构体

```c
/*封装线程池中的对象需要执行的任务对象*/
typedef struct task {
    void *(*run)(void *args);  //函数指针，需要执行的任务
    void *argv;                //参数
    struct task *next;         //任务队列中下一个任务
}task_t;


/*下面是线程池结构体*/
typedef struct threadpool {
    condition_t ready;    //状态量
    task_t *first;        //任务队列中第一个任务
    task_t *last;         //任务队列中最后一个任务
    int counter;          //线程池中已有线程数
    int idle;             //线程池中空闲线程数
    int max_threads;      //线程池最大线程数
    int quit;             //是否退出标志 1/0
}threadpool_t;
```

**状态量：**状态量就是互斥锁与条件变量，在多线程环境中，线程池就相当于临界资源，所以就需要条件变量和互斥锁来保证每一个线程访问临界资源的安全高效。 

**first/last指针：**就是线程池中维护的任务队列，在使用线程池时，只需要向队列中添加任务就行，线程池中的空闲线程会”主动“执行任务。 

**max_threads：**线程池最大线程数，如果向任务队列中添加新任务，此时，线程池中并没有空闲线程来执行任务，就需要创建新的线程执行，但是，如果此时已有线程数已经达到线程池的最大线程数，那么就会失败返回。

#### 具体代码实现：

用条件变量和互斥锁封装的状态，用来保证线程池的安全

#### condition.h

```c
/*************************************************************************
	> File Name: condition.h
	> Author: zhengdongqi
	> Mail: 1821260963@qq.com
	> Created Time: 四  3/ 7 13:54:38 2019
 ************************************************************************/

#ifndef _CONDITION_H
#define _CONDITION_H

#include <pthread.h>
#include <stdio.h>

/*封装一个互斥量和条件变量作为状态*/
typedef struct condition {  
    pthread_mutex_t pmutex;//互斥量
    pthread_cond_t pcond;//条件变量
}condition_t;

/*对状态的操作函数*/
/*初始化*/
int condition_init(condition_t *cond);
/*加锁*/
int condition_lock(condition_t *cond);
/*解锁*/
int condition_unlock(condition_t *cond);
/*条件等待*/
int condition_wait(condition_t *cond);
/*计时等待*/
int condition_timedwait(condition_t *cond, const struct timespec *abstime);
/*唤醒一个睡眠线程*/
int condition_signal(condition_t* cond);
/*唤醒所有睡眠线程*/
int condition_broadcast(condition_t *cond);
/*销毁互斥锁和条件变量*/
int condition_destroy(condition_t *cond);

#endif
```

#### condition.c

```c
/*************************************************************************
	> File Name: condition.c
	> Author: zhengdongqi
	> Mail: 
	> Created Time: 四  3/ 7 13:57:50 2019
 ************************************************************************/

#include "condition.h"

/*初始化*/
int condition_init(condition_t *cond) {
    int status;
    if((status = pthread_mutex_init(&cond->pmutex, NULL)))
        return status;

    if((status = pthread_cond_init(&cond->pcond, NULL)))
        return status;
    return 0;
}
/*加锁*/
int condition_lock(condition_t *cond) {
    return pthread_mutex_lock(&cond->pmutex);
}
/*解锁*/
int condition_unlock(condition_t *cond) {
    return pthread_mutex_unlock(&cond->pmutex);
}
/*条件等待*/
int condition_wait(condition_t *cond) {
    return pthread_cond_wait(&cond->pcond, &cond->pmutex);
}
/*计时等待*/
int condition_timedwait(condition_t *cond, const struct timespec *abstime) {
    return pthread_cond_timedwait(&cond->pcond, &cond->pmutex, abstime);
}
/*唤醒一个睡眠线程*/
int condition_signal(condition_t *cond) {
    return pthread_cond_signal(&cond->pcond);
}

/*唤醒所有睡眠线程*/
int condition_broadcast(condition_t *cond) {
    return pthread_cond_broadcast(&cond->pcond);
}

/*销毁互斥锁和条件变量*/
int condition_destroy(condition_t *cond) {
    int status;
    if((status = pthread_mutex_destroy(&cond->pmutex)))
        return status;

    if((status = pthread_cond_destroy(&cond->pcond)))
        return status;
    return 0;
}
```

线程池进行线程调用

#### threadpool.h

```c
/*************************************************************************
	> File Name: threadpool.h
	> Author: zhengdongqi
	> Mail: 
	> Created Time: 四  3/ 7 14:00:31 2019
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

//需要引入状态的头文件--条件变量和互斥锁
#include "../condition/condition.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/*封装线程池中的对象需要执行的任务对象*/
typedef struct task {
    void *(*run)(void *args);  //函数指针，需要执行的任务
    void *argv;                //参数
    struct task *next;         //任务队列中下一个任务
}task_t;


/*下面是线程池结构体*/
typedef struct threadpool {
    condition_t ready;    //状态量
    task_t *first;        //任务队列中第一个任务
    task_t *last;         //任务队列中最后一个任务
    int counter;          //线程池中已有线程数
    int idle;             //线程池中空闲线程数
    int max_threads;      //线程池最大线程数
    int quit;             //是否退出标志 1/0
}threadpool_t;


/*线程池初始化*/
void threadpool_init(threadpool_t *pool, int idle_threads, int max_threads);

/*往线程池中加入任务*/
int threadpool_add_task(threadpool_t *pool, void *(*run)(void *args), void *argv);

/*摧毁线程池*/
void threadpool_destroy(threadpool_t *pool);

#endif
```

#### threadpool.c

```c
/*************************************************************************
	> File Name: threadpool.c
	> Author: zhengdongqi
	> Mail: 
	> Created Time: 四  3/ 7 14:02:02 2019
 ************************************************************************/
#include "threadpool.h"

/*线程池中创建的线程执行*/
void *thread_routine(void *argv) {
    struct timespec abstime;//时间结构体
    int timeout;
    DBG("threadpool.c->thread_routine\033[35m线程%d开始了！\033[0m\n", (int)pthread_self());
    threadpool_t *pool = (threadpool_t *)argv;

    //死循环使线程池中空闲的线程可以复用
    while(1) {   
        timeout = 0;

        //访问线程池之前需要加锁
        condition_lock(&pool->ready);

        //空闲
        pool->idle++;

        //任务队列没有任务到来并且没有收到线程池销毁通知, 线程阻塞等待(进入这里面都是空闲线程，等待被唤醒)
        while(pool->first == NULL && !pool->quit) {
            DBG("threadpool.c->thread_routine\033[35m线程%d等待中！\033[0m\n", (int)pthread_self());
            //获取当前时间，并加上等待时间， 从而设置进程的超时睡眠时间
            clock_gettime(CLOCK_REALTIME, &abstime);  
            abstime.tv_sec += 2;
            int status;
            status = condition_timedwait(&pool->ready, &abstime);  //该函数会解锁，允许其他线程访问，当被唤醒时，加锁
            if(status == ETIMEDOUT) {
                DBG("threadpool.c->thread_routine\033[35m线程%d等待超时！\033[0m\n", (int)pthread_self());
                timeout = 1;
                break;
            }   
        }   

        pool->idle--;
        if(pool->first != NULL) {   
            //取出任务队列最前的任务，移除任务，并执行任务
            task_t *t = pool->first;
            pool->first = t->next;
            //由于任务执行需要消耗时间，先解锁让其他线程访问线程池
            condition_unlock(&pool->ready);
            //执行任务
            t->run(t->argv);
            //执行完任务释放内存
            free(t);
            //重新加锁
            condition_lock(&pool->ready);
        }   

        //退出线程池--销毁当前线程
        if(pool->quit && pool->first == NULL) {
            pool->counter--;//当前工作的线程数-1

            //若线程池中没有线程，唤醒等待线程（主线程--销毁线程池的线程）全部任务已经完成
            if(pool->counter == 0) {
                condition_signal(&pool->ready);
            }
            condition_unlock(&pool->ready);
            break;
        }
        //超时，说明线程没有任务可以执行, 跳出销毁线程
        if(timeout == 1) {
            pool->counter--;//当前工作的线程数-1
            condition_unlock(&pool->ready);
            break;
        }

        condition_unlock(&pool->ready);
    }
    DBG("threadpool.c->thread_routine\033[35m线程%d退出！\033[0m\n", (int)pthread_self());
    return NULL;
}


/*线程池初始化*/
void threadpool_init(threadpool_t *pool, int idle_threads, int max_threads) {
    
    condition_init(&pool->ready);
    pool->first = NULL;
    pool->last = NULL;
    pool->counter = 0;
    pool->idle = 0;
    pool->max_threads = max_threads;
    pool->quit = 0;

    //创建空闲线程
    int i = 0;
    for(; i < idle_threads; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, pool);
        pool->counter++;//已有线程数+1
   }
}

/*增加一个任务到线程池*/
int threadpool_add_task(threadpool_t *pool, void *(*run)(void *args), void *argv) {
    //产生一个新的任务
    task_t *newtask = (task_t *)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->argv = argv;
    newtask->next = NULL;//新加的任务放在队列尾端

    //线程池的状态被多个线程共享，操作前需要加锁
    condition_lock(&pool->ready);

    //第一个任务加入
    if(pool->first == NULL) {
        pool->first = newtask;
    } else {
        pool->last->next = newtask;
    }
    pool->last = newtask;  //队列尾指向新加入的线程

    //线程池中有线程空闲，唤醒处于等待状态的线程(因为在等待期间会释放互斥锁)
    if(pool->idle > 0) {
        condition_signal(&pool->ready);
    } else if (pool->counter < pool->max_threads) {//当前线程池中线程个数没有达到设定的最大值，创建一个新的线程
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, pool);
        pool->counter++;
        DBG("threadpool.c->threadpool_add_task\033[35m线程数量 %d\033[0m\n", pool->counter);
    } else {
        condition_unlock(&pool->ready);
        return -1;
    }
    //结束，访问
    condition_unlock(&pool->ready);
    return 0;
}

/*线程池销毁*/
void threadpool_destroy(threadpool_t *pool) {
    //如果已经调用销毁，直接返回
    if (pool->quit) {
        return ;
    }
    //加锁
    condition_lock(&pool->ready);
    //设置销毁标记为1
    pool->quit = 1;
    //线程池中线程个数大于0
    if (pool->counter > 0) {
        //对于等待的线程，发送信号唤醒
        if (pool->idle > 0) {
            condition_broadcast(&pool->ready);
        }
        //正在执行任务的线程，等待他们结束任务
        while(pool->counter) {
            condition_wait(&pool->ready);
        }
    }
    condition_unlock(&pool->ready);
    condition_destroy(&pool->ready);
}              
```

#### 测试代码test.c

```c
/*************************************************************************
	> File Name: test.c
	> Author: zhengdongqi
	> Mail: 
	> Created Time: 三  4/ 3 17:29:23 2019
 ************************************************************************/

#include<stdio.h>
#include "./threadpool/threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void *mytask(void *arg) {
    printf("thread %d is working on task %d\n", (int)pthread_self(), (int)arg);
    sleep(1);
    return NULL;
}
//测试代码
int main()  {
    threadpool_t pool;
    //初始化线程池，创建2个空闲线程，最多5个线程
    threadpool_init(&pool, 2, 5); 
    int i;
    //创建十个任务
    for(i = 0; i < 10; i++) {   
        threadpool_add_task(&pool, mytask, (void *)&i);
    }   
    threadpool_destroy(&pool);
    return 0;
}
```

#### makefile

```bash
CC = gcc
CFLAGS =  -std=c99 -Wall -g -D _DEBUG 
OBJS = test.o condition.o threadpool.o

.PHONY: clean

./a.out : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) 

test.o : test.c
	$(CC) -c $(CFLAGS) test.c

condition.o : ./condition/condition.c ./condition/condition.h 
	$(CC) -c $(CFLAGS) ./condition/condition.c 

threadpool.o : ./threadpool/threadpool.c ./threadpool/threadpool.h 
	$(CC) -c $(CFLAGS) ./threadpool/threadpool.c

clean:
	rm -rf *.o ./a.out
```

## 执行结果

![threadpool-test](../threadpool/pic/threadpool-test.png)

# 线程池机制

![threadpool-jz](../threadpool/pic/threadpool-jz.png)


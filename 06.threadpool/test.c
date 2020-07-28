/*************************************************************************
	> File Name: test.c
	> Author: zhengdongqi
	> Mail: 1821260963@qq.com
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


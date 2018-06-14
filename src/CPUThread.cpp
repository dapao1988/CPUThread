//============================================================================
// Name        : CPUThread.cpp
// Author      : wenbing.wang
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>

typedef struct MY_THREAD_T {
    pthread_t tid;
    int index;
}MY_THREAD;

#define MAX_NUM 8
void *myfun(void *arg)
{
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];
    int j;
    MY_THREAD *myThread = (MY_THREAD *) arg;
    int i = myThread->index;

    assert(myThread != NULL);

    int num = sysconf(_SC_NPROCESSORS_CONF);
    //printf("system has %d processor(s)\n", num);

    CPU_ZERO(&get);
    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
        fprintf(stderr, "get thread affinity failed\n");
    }
    printf("This is thread-%d, (int)pthread_self():%d,myThread->tid:%d, :0x%x\n", myThread->index, (int)pthread_self(),myThread->tid, get);


    CPU_ZERO(&mask);
    CPU_SET(i, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
    }

    CPU_ZERO(&get);
    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
        fprintf(stderr, "get thread affinity failed\n");
    }

    //printf("myfun get:0x%x\n", get);

    for (j = 0; j < num; j++) {
        if (CPU_ISSET(j, &get)) {
            printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
        }
    }
    j = 0;

    pthread_yield();
    //usleep(1000);
    pthread_exit(NULL);

    return NULL;
}

void *testfun(void *arg)
{
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];
    int j;
    MY_THREAD *myThread = (MY_THREAD *) arg;

    assert(myThread != NULL);
    CPU_ZERO(&get);

    if (pthread_getaffinity_np(myThread->tid, sizeof(get), &get) < 0) {
        fprintf(stderr, "get thread affinity failed\n");
    }

    printf("This is thread-%d, (int)pthread_self():%d,myThread->tid:%d, :0x%x\n", myThread->index, (int)pthread_self(),myThread->tid, get);

    for (j = 0; j < MAX_NUM; j++) {
        if (CPU_ISSET(j, &get)) {
            printf("thread%d %d is running in processor %d\n", myThread->index, (int)pthread_self(), j);
            //break;
        } else {
            printf("not run on CPU-%d\n", j);
        }
    }

    while (j++ < 100000000) {
        memset(buf, 0, sizeof(buf));
    }
    pthread_yield();
    pthread_exit(NULL);

    return NULL;
}


#define DISPATCH_8
int main(int argc, char *argv[])
{
    MY_THREAD myThread[8];


    for(int i=0;i<MAX_NUM;i++) {
        if (pthread_create(&myThread[i].tid, NULL, myfun, &myThread[i]) != 0) {
            fprintf(stderr, "thread create failed\n");
            return -1;
        }
        myThread[i].index = i;
    }

    for(int i=0;i<MAX_NUM;i++) {
        pthread_join(myThread[i].tid, NULL);
    }

    printf("main exit\n");
    return 0;
}

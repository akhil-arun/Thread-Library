/* EXPECTED OUPUT
 Entering thread1 loop
 Entering thread2 loop
 Thread 3
 Thread 2
 Thread 1
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

volatile bool i, j;

void thread3(void *arg)
{
    (void)arg;
    printf("Thread 3 \n");
    i = 0;
}

void thread2(void *arg)
{
    (void)arg;
    printf("Entering thread2 loop\n");
    while (1){
        if (i == 0)
            break;
    }
    printf("Thread 2\n");
    j = 0;
}

void thread1(void *arg)
{
    (void)arg;

    uthread_create(thread2, NULL);
    uthread_create(thread3, NULL);
    printf("Entering thread1 loop\n");
    while (1){
        if (j == 0)
            break;
    }
    printf("Thread 1\n");
}

int main(void)
{
    j = 1;
    i = 1;
    uthread_run(true, thread1, NULL);
    return 0;
}

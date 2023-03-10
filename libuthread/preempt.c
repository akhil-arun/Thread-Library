#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */


#define HZ 100
// Amount of time before timer goes off in microseconds
#define IN_MICRO 10000

sigset_t ss;

void alarm_handler(int signum)
{
    if (signum == SIGVTALRM)
        uthread_yield();
}

void preempt_disable(void)
{
    sigprocmask(SIG_BLOCK, &ss, NULL);
}

void preempt_enable(void)
{
    sigprocmask(SIG_UNBLOCK, &ss, NULL);
}

void preempt_start(bool preempt)
{

    sigemptyset(&ss);
    sigaddset(&ss, SIGVTALRM);

    if (!preempt)
        return;

    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGVTALRM, &sa, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = IN_MICRO;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = IN_MICRO;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
    setitimer(ITIMER_VIRTUAL, NULL, NULL);
    sigaction(SIGVTALRM, NULL, NULL);
}

#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

#define RUNNING 1
#define READY 2
#define BLOCKED 3
#define ZOMBIE 4

struct uthread_tcb
{
    int state;
    uthread_ctx_t ctx;
    void *stack;
};

// Ready queue has ready threads, and zombie queue has finished threads to be destroyed 
queue_t ready, zombie;

// Current is the thread that is running, and idle is the idle thread
struct uthread_tcb *current, *idle;

struct uthread_tcb *uthread_current(void)
{
    return current;
}

void uthread_yield(void)
{
    preempt_disable();
    struct uthread_tcb *yield_thread = current;

    // Add thread back to ready queue if the current thread's state is "RUNNING"
    if (yield_thread->state == RUNNING){
        yield_thread->state = READY;
        queue_enqueue(ready, yield_thread);
    }

    // Get the next ready thread and make it the current thread
    queue_dequeue(ready, (void **) &current);
    current->state = RUNNING;

    preempt_enable();
    
    // Context switch from the yielding thread to the next thread for execution
    uthread_ctx_switch(&(yield_thread->ctx), &(current->ctx));
}

void uthread_exit(void)
{
    preempt_disable();

    // Add the current to the zombie queue, and yield to switch contexts 
    current->state = ZOMBIE;
    queue_enqueue(zombie, current);
    uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{

    // Allocate space for new TCB and check for mem. allocation errors
    preempt_disable();
    struct uthread_tcb *new_tcb = malloc(sizeof(struct uthread_tcb));
    if (new_tcb == NULL)
        return -1;

    preempt_enable();

    //Allocate space for the TCB's stack and check for mem. allocation errors
    preempt_disable();
    new_tcb->stack = uthread_ctx_alloc_stack();
    if (new_tcb->stack == NULL){
        free(new_tcb);
        return -1;
    }
    preempt_enable();
    
    // Initialize the TCB's context object and check if the init function had errors
    preempt_disable();
    int retval = uthread_ctx_init(&(new_tcb->ctx), new_tcb->stack, func, arg);
    if (retval == -1){
        uthread_ctx_destroy_stack(new_tcb->stack);
        free(new_tcb);
        return -1;
    }

    new_tcb->state = READY;
    queue_enqueue(ready, new_tcb);
    
    preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    // Creating the zombie/ready queues and checking for errors
    ready = queue_create();
    zombie = queue_create();
    if (ready == NULL || zombie == NULL)
        return -1;

    // Intialization of idle thread, and it is set as the "running" thread
    idle = malloc(sizeof(struct uthread_tcb));
    if (idle == NULL)
        return -1;
    idle->state = RUNNING;
    current = idle;

    preempt_start(preempt);

    // Creating the first thread and checking for errors
    int retval = uthread_create(func, arg);
    if(retval == -1)
        return -1;
    
    // Idle Loop until only idle thread remains
    while (queue_length(ready) > 0){
        uthread_yield();
        
        // Destroying all threads in the zombie queue
        while(queue_length(zombie) > 0){
            preempt_disable();
            
            struct uthread_tcb *delete_tcb;
            queue_dequeue(zombie, (void**) &delete_tcb);
            uthread_ctx_destroy_stack(delete_tcb->stack);
            free(delete_tcb);
            
            preempt_enable();
        }
    }
    
    if(preempt)
        preempt_stop();
    
    // Destroy the queues and idle thread
    queue_destroy(zombie);
    queue_destroy(ready);
    free(idle);
    return 0;
}

void uthread_block(void)
{
    current->state = BLOCKED;
    uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    uthread->state = READY;
    queue_enqueue(ready, uthread);
}

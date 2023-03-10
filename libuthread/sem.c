#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore
{
    queue_t blocked;
    int count;
};

sem_t sem_create(size_t count)
{
    sem_t new_sem = malloc(sizeof(struct semaphore));
    if(!new_sem)
        return NULL;

    new_sem->count = count;
    new_sem->blocked = queue_create();
    if (new_sem->blocked == NULL) {
        free(new_sem);
        return NULL;
    }
    
    return new_sem;
}

int sem_destroy(sem_t sem)
{
    if(sem == NULL || queue_destroy(sem->blocked) == -1)
        return -1;

    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    if(sem == NULL)
        return -1;

    preempt_disable();

    // Stay inside loop until a resource becomes available
    while(sem->count == 0){
        queue_enqueue(sem->blocked, uthread_current());
        uthread_block();
        preempt_disable();
    }
    
    sem->count -= 1;

    preempt_enable();
    return 0;
}

int sem_up(sem_t sem)
{
    if(sem == NULL)
        return -1;

    preempt_disable();

    // If there are threads waiting on resource, then unblock a thread
    if(queue_length(sem->blocked) > 0){
        struct uthread_tcb *unblock_tcb;
        queue_dequeue(sem->blocked, (void**) &unblock_tcb);
        uthread_unblock(unblock_tcb);
    }

    sem->count += 1;

    preempt_enable();
    return 0;
}

Github Link: https://github.com/akhiller30/Thread-Library
# Introduction
The purpose of this project is to understand how threads work by implementing
a user-level thread library (libuthread).

## Features:
1. Queue API
2. Uthread API
3. Thread synchornization API using semaphores (Semaphore API)
4. Interrupt-based scheduler (Preemption API)

# Phase 1: Queue API
A queue is a FIFO(first-in-first-out) data structure such that the first element
enqueued is always the first element to be dequeued. For our queue 
implementation, we decided to use a linked-list style implementation. This was
done to prevent the common pitfalls of an array style implementation such as 
having to allocate more space when the queue reaches its capacity, and having to 
slide elements down when dequeueing or deleting an element.

## Node Struct:
```c
struct node{
    void *data;
    struct node *next;
};
```
Each element(node) in the queue contains a pointer to the data, and a pointer to
the next element in the queue. A void pointer is used for the data because it 
will provide the API with greater versatility such that any data type or struct
can be added to queue.

## Queue Struct:
```c
struct queue{
    node_t head;
    node_t tail;
    int length;
};
```
The queue struct contains the head(first) node, tail(last) node, and the length 
of the queue.

## Queue API Functions:
### `queue_create()`
Dynamically allocate space for the queue struct, and sets all the members to 
their default values: NULL for the nodes and 0 for the length.
### `queue_destroy()`
Frees the dynamically allocated space if the queue is empty or 
`queue->length == 0`. Hence this will be a O(1) operation.
### `queue_enqueue()`
Allocates space for the new node, sets the next pointer for the node to NULL,
and sets the node's data pointer to the data pointer passed in as a parameter.
This new node will become the previous tail's next node, and the new node will
become the new tail. Edge case: if the queue's length is zero, then we set the 
queue's head and tail equal to the new node. This is a O(1) operation.
### `queue_dequeue()`
Removes the head node from the queue, and sets `head->next` as the head node.
This function also places the data from the dequeued node into the dereferenced
`void ** data` that is passed in as parameter. This is a O(1) operation. 
### `queue_delete()`
Iterates through the queue and checks to see if the data that is passed in as
a parameter is currently in the queue. If the specified data is in the queue 
then the node is removed, and the previous node's next pointer is adjusted. Edge 
case: if the data paramater is equal to the head's data member, then we simply 
call the `queue_dequeue()` function. This is a O(N) operation.
### `queue_iterate()`
Iterates through the queue and calls a function that is passed in as a parameter
on each of the nodes in the queue. Since there is a chance that the function 
could delete the node, we store the node's next member before calling the 
function on the node. This is a O(N) operation.
### `queue_length()`
Returns the queue's length data member. The queue's length is updated when 
calling the enqueue, dequeue, or delete functions. This is a O(1) operation.

## Testing
We created a file queue_tester.c that uses unit testing to test that the queue 
works properly under all scenarios including edge cases. In this file we defined
10 test cases that stresses the functions in the API, and based on the 
results of our testing, we believe that the queue API works as expected.

# Phase 2: Uthread API
This API provides an multithreading interface that allows a user to create, run,
terminate, and manipulate threads. Threads have their own flow of execution, but
have a single adress space, so they share the global variables, heap, and etc.

## Uthread_tcb struct:
```c
struct uthread_tcb{
    int state;
    uthread_ctx_t ctx;
    void *stack;
};
```
Each thread has a tcb that keeps track of their state (running, ready, blocked, 
or, zombie), a context to keep track of a thread's execution context, and a
stack.

## Uthread API Functions:
### `uthread_exit()`
Sets the state of the current thread to zombie, adds the thread to the zombie
queue, and calls `uthread_yield`.
### `uthread_yield()`
Adds the current thread to the ready queue if the current's thread state is
running. Dequeues a thread from the ready queue, and sets that thread as the new
current thread. Lastly, `uthread_ctx_switch()` is called to switch to the new 
current thread.
### `uthread_create()`
Initializes all of the relevant members in the uthread_tcb struct, and the new
tcb is enqueued to the ready queue if there are no errors.
### `uthread_run()`
Initializes the ready and zombie queues, the idle thread, and calls 
`uthread_create()` to add the first thread to the ready queue. There is an 
idle loop where `uthread_yield()` will be called while the ready queue's length
is greater than zero. In addition, all threads that are in the zombie queue will
be destroyed during each iteration of the loop.
### `uthread_block()`
Sets the current's thread state to blocked and calls `uthread_yield()`.
### `uthread_unblock()`
Sets the tcb that was passed in as a parameter's state to ready, and it is added
to the ready queue.

# Phase 3: Semaphore API
A semaphore is a type of lock that kind be used for different types of
synchronization, and they are a way to control access to a shared resource used 
by threads. Semaphores have an internal count, and a queue of threads waiting
to access the resource.

## Semaphore struct
```c
struct semaphore{
    queue_t blocked;
    int count;
};
```
The count variable keeps track of the number of the resource that is available,
and the blocked queue has all the threads that are waiting on the resource.

## Semaphore API Functions
### `sem_create()`
Allocates space for the semaphore struct and intializes the struct's count to 
the parameter that was passed in.
### `sem_destroy()`
Destroys the semaphore if the blocked queue is empty.
### `sem_down()`
If the sem's count is zero then the function will enter a loop where the
current/running queue will be added to the blocked queue, and the function
`uthread_block()` will be called. The sem's count will be decremented.
#### Edge Case Protection
In our implementation we orginally used an if statement instead of a while loop; 
however, this failed to capture the edge case. With a while loop if another 
thread claims the resource before the unblocked thread can run again, the thread 
will be blocked again when it is yielded back to.
### `sem_up()`
If the blocked queue is not empty, then a thread will be dequeued and
`uthread_unblock()` will be called. The sem's count will be incremented.

# Phase 4: Preemption API

## Preempt API Functions
### `preempt_disable()`
Disables the timer by using SIG_BLOCK in the sigpromask() function to ignore 
the alarm.
### `preempt_enable()`
Enables the timer by using SIG_UNBLOCK in the sigpromask() function to make the 
program receptive to alarm signals.
### `preempt_start()`
Configures a timer that sends out an alarm signal at a frequency of 100HZ. 
Starts by initializing a sigaction struct called "sa" and sets its flags 
and mask attributes to zero. The handler is redirected to an alarm handler
method which calls uthread_yield. The sigaction() method is called on sa to 
set it to SIGVTALARM. An instance of the itimerval struct is created called
timer, with the wait period and interval period set to 10000 microseconds, which
corresponds to a frequency of 100HZ. The setitimer() method is called with
the struct itimerval to start the 100HZ timer.
### `preempt_stop()`
Stops the timer by setting SIGVTALARM to NULL using the sigaction() method, 
before also setting the timer to NULL using the setitimer() method.

## Testing
To test preemption we came up with a scenario where the threads will run forever
until a specific boolean value is met in the 'test_preempt.c' file. In this file
we created three different threads; the first two threads will loop forever 
because the boolean expressions evaluate to false. This will result in 
the program being preempted, and thread3 will get run. In thread3 we change the
global bool variable, so now when thread2 is run, we will break out of the loop.
A similar process is repeated after the thread2 loop, so that thread1 will
finish running. As a result, the threads will be printed in reverse numerical
order. We know that preemption works because we did not call 'uthread_yield()'
inside our tester file.

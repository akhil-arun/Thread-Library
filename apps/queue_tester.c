#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                 \
    do                                      \
    {                                       \
        printf("ASSERT: " #assert " ... "); \
        if (assert)                         \
        {                                   \
            printf("PASS\n");               \
        }                                   \
        else                                \
        {                                   \
            printf("FAIL\n");               \
            exit(1);                        \
        }                                   \
    } while (0)

/* Create */
void test_create(void)
{
    fprintf(stderr, "*** TEST create ***\n");

    TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
    int data = 3, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_simple ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data);
}

void test_destroy(void)
{
    int data = 0, *ptr;
    queue_t q;
    fprintf(stderr, "*** TEST destroy ***\n");
   
    q = queue_create();
    queue_enqueue(q, &data);
    TEST_ASSERT(queue_destroy(q) == -1);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(queue_destroy(q) == 0);
}

static void iterator_inc(queue_t q, void *data)
{
    int *a = (int *)data;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += 1;
}

void test_iterator(void)
{
    fprintf(stderr, "*** TEST iterator ***\n");
   
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(data[5] == 42);
    TEST_ASSERT(queue_length(q) == 9);
}

void test_delete(void)
{
    int data[] = {1, 2, 3, 8, 10};
    queue_t q;
    fprintf(stderr, "*** TEST destroy ***\n");
   
    q = queue_create();
    for (int i = 0; i < 3; ++i)
        queue_enqueue(q, &data[i]);

    TEST_ASSERT(queue_length(q) == 3);

    
    queue_delete(q, &data[1]);
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(data[1] == 2);
    TEST_ASSERT(data[2] == 4);
    TEST_ASSERT(queue_length(q) == 2);

    
    queue_delete(q, &data[0]);
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(data[2] == 5);
    TEST_ASSERT(queue_length(q) == 1);

    queue_enqueue(q, &data[3]);
    queue_enqueue(q, &data[4]);
    TEST_ASSERT(queue_delete(q, &data[0]) == -1);
}

void test_enqueue_dequeue(void)
{
    int data[] = {0, 1, 2};
    int *ptr;
    fprintf(stderr, "*** TEST enqueue/destroy ***\n");
    queue_t q = queue_create();

    TEST_ASSERT(queue_dequeue(q, (void **)&ptr) == -1);

    queue_enqueue(q, &data[0]);
    queue_enqueue(q, &data[1]);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data[0]);

    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data[1]);

    queue_enqueue(q, &data[2]);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data[2]);
    TEST_ASSERT(queue_length(q) == 0);
    TEST_ASSERT(queue_destroy(q) == 0);
}

void test_deleteFromEnd_enqueue(void)
{
    int data[] = {0, 1, 2};
    fprintf(stderr, "*** TEST delete newest node/enqueue ***\n");
    queue_t q = queue_create();

    queue_enqueue(q, &data[0]);
    queue_enqueue(q, &data[1]);
    queue_delete(q, &data[1]);

    int *ptr;
    queue_enqueue(q, &data[2]);
    queue_dequeue(q, (void **)&ptr);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data[2]);
}

void test_enqueue_dequeue_errors(void)
{
    queue_t q = NULL;
    int ptr;
    int data[] = {0, 1};
    fprintf(stderr, "*** TEST enqueue dequeue errors ***\n");
    
    int retval = queue_enqueue(q, &data[0]);
    TEST_ASSERT(retval == -1);
    
    q = queue_create();
    retval = queue_enqueue(q, NULL);
    TEST_ASSERT(retval == -1);
    
    queue_enqueue(q, &data[0]);
    queue_dequeue(q, (void**) &ptr);
    retval = queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(retval == -1);
}

void test_delete_not_found(void)
{
    queue_t q;
    int data[] = {0, 1, 2};
    fprintf(stderr, "*** TEST delete not found***\n");
    
    q = queue_create();
    queue_enqueue(q, &data[0]);
    queue_enqueue(q, &data[1]);
    int retval = queue_delete(q, &data[2]);
    TEST_ASSERT(retval == -1);
}

void test_iterate_errors(void)
{
    queue_t q;
    int data[] = {0, 1, 2};
    fprintf(stderr, "**** TEST iterate errors ***\n");

    q = queue_create();
    queue_enqueue(q, &data[0]);
    queue_enqueue(q, &data[1]);
    int retval = queue_iterate(q, NULL);
    TEST_ASSERT(retval == -1);
}

int main(void)
{
    test_create();
    test_queue_simple();
    test_iterator();
    test_destroy();
    test_delete();
    test_enqueue_dequeue();
    test_deleteFromEnd_enqueue();
    test_enqueue_dequeue_errors();
    test_delete_not_found();
    test_iterate_errors();

    return 0;
}
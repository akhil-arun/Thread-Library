#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct node
{
    void *data;
    struct node *next;
};
typedef struct node *node_t;

struct queue
{
    node_t head;
    node_t tail;
    int length;
};


queue_t queue_create(void)
{
    queue_t new_queue = malloc(sizeof(struct queue));
    if(new_queue == NULL)
        return NULL;
    
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->length = 0;
    return new_queue;
    
}

int queue_destroy(queue_t queue)
{
    if (queue == NULL|| queue->length != 0)
        return -1;
    
    free(queue);
    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if(queue == NULL || data == NULL)
        return -1;

    node_t new_node = malloc(sizeof(struct node));
    if(new_node == NULL)
        return -1;

    new_node->data = data;
    new_node->next = NULL;

    // There are no nodes in the queue currently
    if (queue->length == 0){
        queue->head = new_node;
        queue->tail = new_node;
    }
    
    // Normal Case
    else{
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    queue->length += 1;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if(queue == NULL || queue->length == 0 || data == NULL)
        return -1;

    node_t removed_node = queue->head;
    *data = removed_node->data;
    queue->head = removed_node->next;
    free(removed_node);
    queue->length -= 1;
    return 0;
}

int queue_delete(queue_t queue, void *data)
{
    if(queue == NULL || queue->length == 0 || data == NULL)
        return -1;
    
    // Data is equal to the head's data
    if(data == queue->head->data){
        node_t removed_node;
        queue_dequeue(queue, (void **) &removed_node);
        return 0;
    }
    node_t previous = queue->head;
    node_t current = queue->head->next;
    
    // Data is potentially in the nodes after the head node
    while(current != NULL){
        if(current->data == data){
            node_t removed_node = current;
            previous->next = current->next;
            
            // If tail is deleted then the tail pointer is updated
            if(data == queue->tail->data)
                queue->tail = previous;

            queue->length -= 1;
            free(removed_node);
            return 0;
        }
        
        else{
            previous = current;
            current = current->next;
        }
    }

    return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
    if(queue == NULL || func == NULL)
        return -1;
    
    node_t itr = queue->head;
    
    // Call the function on each node, store the next node in case it gets deleted
    while(itr){
        node_t next_node = itr->next;
        func(queue, itr->data);
        itr = next_node;
    }

    return 0;
}

int queue_length(queue_t queue)
{
    if(queue == NULL)
        return -1;

    return queue->length;
}
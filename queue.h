#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Queue{
    int32_t capacity;
    int32_t used;
    char *data;
    char type_size;
} Queue;

void
queue_append(Queue *queue, void* thing){
    if (queue->used < queue->capacity){
        memcpy(&queue->data[queue->used * queue->type_size], thing, queue->type_size);
        ++queue->used;
    }
    else{
        queue->data = (char*)realloc(queue->data, queue->capacity * queue->type_size * 2);
        memcpy(&queue->data[queue->used * queue->type_size], thing, queue->type_size);
        ++queue->used;
        queue->capacity *= 2;
    }
}

void
enqueue(Queue *queue, void* thing){
    if (queue->used < queue->capacity){
        memcpy(&queue->data[queue->type_size], &queue->data[0], queue->type_size * queue->used++);
        memcpy(&queue->data[0], thing, queue->type_size);
    }
    else{
        queue->data = (char*)realloc(queue->data, queue->capacity * queue->type_size * 2);
        memcpy(&queue->data[queue->type_size], &queue->data[0], queue->type_size * queue->used++);
        memcpy(&queue->data[0], thing, queue->type_size);
        queue->capacity *= 2;
    }
}

void
queue_pop(Queue *queue, void* dest){
    if (queue->used | 0){
        memcpy(dest, &queue->data[0], queue->type_size);
        memcpy(&queue->data[0], &queue->data[1 * queue->type_size], --queue->used * queue->type_size);
    } else{
        dest = 0;
    }
}

void
queue_insert(Queue *queue, void* thing, int32_t index){
    if (queue->used < queue->capacity){
        memcpy(&queue->data[(index + 1) * queue->type_size], &queue->data[index * queue->type_size], queue->type_size *  (queue->used - index));
        memcpy(&queue->data[index * queue->type_size], thing, queue->type_size);
        ++queue->used;
    }
    else{
        queue->data = (char*)realloc(queue->data, queue->capacity * queue->type_size * 2);
        memcpy(&queue->data[(index + 1) * queue->type_size], &queue->data[index * queue->type_size], queue->type_size *  (queue->used - index));
        memcpy(&queue->data[index * queue->type_size], thing, queue->type_size);
        ++queue->used;
        queue->capacity *= 2;
    }
}

void
queue_delete(Queue *queue, int32_t index){
    if (queue->used | 0){
        memcpy(&queue->data[index * queue->type_size], &queue->data[(index + 1) * queue->type_size], queue->type_size *  (queue->used-- - index));
    }
}

#endif

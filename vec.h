#ifndef VEC_H
#define VEC_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Vec{
    size_t capacity;
    size_t used;
    char *data;
    char type_size;
} Vec;

void vec_append(Vec *vec, void* thing){
    if (vec->used < vec->capacity){
        memcpy(&vec->data[vec->used * vec->type_size], thing, vec->type_size);
        ++vec->used;
    }
    else{
        vec->data = (char*)realloc(vec->data, vec->capacity * vec->type_size * 2);
        memcpy(&vec->data[vec->used * vec->type_size], thing, vec->type_size);
        ++vec->used;
        vec->capacity *= 2;
    }
}

void vec_pop(Vec *vec, void* dest){
    if (vec->used | 0){
        memcpy(dest, &vec->data[--vec->used * vec->type_size], vec->type_size);
    }
}

void vec_insert(Vec *vec, void* thing, size_t index){
    if (vec->used < vec->capacity){
        memcpy(&vec->data[(index + 1) * vec->type_size], &vec->data[index * vec->type_size], vec->type_size *  (vec->used - index));
        memcpy(&vec->data[index * vec->type_size], thing, vec->type_size);
        ++vec->used;
    }
    else{
        vec->data = (char*)realloc(vec->data, vec->capacity * vec->type_size * 2);
        memcpy(&vec->data[(index + 1) * vec->type_size], &vec->data[index * vec->type_size], vec->type_size *  (vec->used - index));
        memcpy(&vec->data[index * vec->type_size], thing, vec->type_size);
        ++vec->used;
        vec->capacity *= 2;
    }
}

Vec* vec_create(size_t initial_capacity, size_t type_size) {
    Vec* vec = (Vec*)malloc(sizeof(Vec));
    if (!vec) return NULL;
    vec->capacity = initial_capacity;
    vec->used = 0;
    vec->type_size = type_size;
    vec->data = (char*)malloc(initial_capacity * type_size);
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void vec_swap(size_t idx_1, size_t idx_2, Vec* vec){
    void* temp = malloc(vec->type_size);

    memcpy(temp, &vec->data[idx_1], vec->type_size);
    memcpy(&vec->data[idx_1], &vec->data[idx_2], vec->type_size);
    memcpy(&vec->data[idx_2], temp, vec->type_size);

    free(temp);
}

void vec_delete(Vec *vec, size_t index){
    if (vec->used | 0){
        memcpy(&vec->data[index * vec->type_size], &vec->data[(index + 1) * vec->type_size], vec->type_size *  (vec->used-- - index));
    }
}

void vec_swap_delete(Vec *vec, size_t index){
    memcpy(&vec->data[index], &vec->data[--vec->used * vec->type_size], vec->type_size);
}

void free_vec(Vec* vec){
    free(vec->data);
}
#endif

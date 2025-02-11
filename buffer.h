#ifndef BUFFER_H
#define BUFFER_H
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Data structures */
typedef struct Buffer {
    char *c;
    size_t len;
    size_t used;
    char state;
} Buffer;

void buf_free(Buffer *buf) {
    free(buf->c);
}

void buf_append(Buffer *buf, char* s, size_t len) {
    if (buf->used + len < buf->len){
        memcpy(&buf->c[buf->used], s, len);
    }
}
#endif

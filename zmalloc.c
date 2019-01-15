// zmalloc - total amount of allocated memory aware version of malloc()


# include <stdlib.h>
# include <string.h>
#include <stdio.h>

static size_t used_memory = 0;

void *zmalloc(size_t size) {
    printf("\nsize %d\n", size);
    void *ptr = malloc(size+sizeof(size_t));

    printf("start pointer %p\n", ptr);
    *((size_t*)ptr) = size;
    used_memory += size + sizeof(size_t);
    printf("contnet pointer %p\n", ptr);
    fflush(stdout);
    return ptr + sizeof(size_t);
}

void *zrealloc(void *ptr, size_t size) {
    void* realptr;
    size_t oldsize;
    void* newptr;

    if (ptr == NULL) return zmalloc(size);
    realptr = ptr - sizeof(size_t);

    newptr = realloc(realptr, size+sizeof(size_t));
    if (!newptr) return NULL;

    newptr += sizeof(size_t);
    oldsize = *((size_t*)realptr);
    used_memory += size - oldsize;
    return newptr;
}

size_t zsize(void* ptr) {
    void* realptr;

    realptr = ptr - sizeof(size_t);

    return *((size_t*)realptr);
}

void zfree(void *ptr) {
    void* realptr;

    if (!ptr) return;
    realptr = ptr - sizeof(size_t);

    used_memory -= *((size_t*)realptr) + sizeof(size_t);
    free(realptr);
}

char *zstrdup(const char *s) {
    size_t l = strlen(s) + 1;

    char *p = zmalloc(l);
    memcpy(p, s, l);

    return p;
}

char zused_memory(void) {
    return used_memory;
}

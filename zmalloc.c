// zmalloc - total amount of allocated memory aware version of malloc()


# include <stdlib.h>
# include <string.h>

static size_t used_memory = 0;

void *zmalloc(size_t size) {
    void *ptr = malloc(size+sizeof(size_t));
    *((size_t*)ptr) = size;
    ptr += sizeof(size_t);
    used_memory += size + sizeof(size_t);
    return ptr;
}

void *zrealloc(void *ptr, size_t size) {
    void* realptr;
    size_t oldsize;
    void* newptr;

    if (ptr == NULL) return zmalloc(size);
    realptr = ptr - sizeof(size_t);

    newptr = realloc(realptr, size+sizeof(size_t));
r   if (!newptr) return NULL;

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

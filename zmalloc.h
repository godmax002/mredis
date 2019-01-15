#ifndef _ZMALLOC_H
#define _ZMALLOC_H

#include <stddef.h>

void* zmalloc(size_t size);
// alloc and clear mem
void* zmalloc(size_t size);
void* zrealloc(void *ptr, size_t size);
size_t zsize(void* ptr);
void* zfree(void* ptr);
char* zstrdup(const char* s);
size_t zused_memory(void);

#endif

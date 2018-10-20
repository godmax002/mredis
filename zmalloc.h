#ifndef _ZMALLOC_H
#define _ZMALLOC_H

void* zmalloc(size_t size);
void* zrealloc(size_t size);
size_t zsize(void* ptr);
void* zfree(void* ptr);
char* zstrdup(const char* s);
size_t zused_memory(void);

#endif

#ifndef __SDS_H
#define __SDS_H

typedef char* sds;

struct sdshdr {
    long len;
    long free;
    char* buf;
}

sds     sdsnewlen(const void* init, size_t initlen);
sds     sdsnew(const char* init);

sds     sdsempty();
void    sdsfree(sds s);

size_t  sdslen(const sds s);
size_t  sdsavail(const sds s);

sds     sdsdup(const sds s);
// concat
sds     sdscatlen(sds s, void* t, size_t len);
sds     sdscat(sds s, char* t);
sds     sdscatprintf(sds s, const char* fmt, ...);

// copy and override
sds     sdscpylen(sds s, char* t, size_t len);
sds     sdscpy(sds s, char* t);

sds     sdstrim(sds s, const char* cset);
void    sdsupdatelen(sds s);
sds     sdsrange(sds s, long start, long end);

int     sdscmp(sds s1, sds s2);
sds*    sdssplitlen(char *s, int len, char *sep, int seplen, int* count);
void    sdstolower(sds s);

#endif

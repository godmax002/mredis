# include "sds.h"
# include "zmalloc.h"

sds     sdsnewlen(const void* init, size_t initlen) {
    struct sdshdr*  sh;

    sh = zmalloc(sizeof(struct sdshdr)+initlen+1);
    if (sh==NULL) return NULL;

    sh->len = initlen;
    sh->free = 0;
    if (initlen) {
        if (init) memcpy(sh->buf, init, initlen);
        else memset(sh->buf, 0, initlen);
    }
    sh->buf[initlen] = '\0';
    return (char*)sh->buf;
}

sds     sdsnew(const char* init) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return sdsnewlen(init, intlen);
}

sds     sdsempty(){
    return sdsnew("");
}

void    sdsfree(sds s) {
    if (s == NULL) return;
    zfree(s-sizeof(struct sdshdr));
}

size_t  sdslen(const sds s);{
    struct sdshdr sh;
    sh = s - sizeof(struct sdshdr);
    return sh->len;
}

size_t  sdsavail(const sds s){
    struct sdshdr sh;
    sh = s - sizeof(struct sdshdr);
    return sh->free;
}

sds     sdsdup(const sds s){
    return sdsnew(s);
}

sds sdsMakeRoom(sds s, size_t addlen){
    struct sdshdr *sh, *newsh ;
    size_t len, newlen;

    if(sdsavail(s) > addlen) return s;
    len = sdslen(s);
    newlen = (len + addlen) * 2;

    sh = (void *)(s - sizeof(struct sdshdr));
    newsh = zrealloc(sh, sizeof(struct sdshdr)+newlen+1);
#ifdef SDS_ABORT_ON_OOM
    if (newsh == NULL) sdsOomAbort();
#else
    if (newsh == NULL) return NULL;
#endif
    newsh->free = newlen - len;
    return newsh->buf;
}

// concat
sds     sdscatlen(sds s, void* t, size_t len){
    struct sdshdr *sh;

    sdsMakeRoom(s, len);
    if(s == NULL) return NULL;
    memcpy(s+sdslen(s), t, len);
    sh = s - sizeof(struct sds);
    sh->len = sh->len + len;
    sh->free = sh->free -len;
    s[sh->len] = '\0';
    return s;
}



    

sds     sdscat(sds s, char* t){
    return sdscatlen(s, t, strlen(t));
}

sds     sdscatprintf(sds s, const char* fmt, ...){
    va_list ap;
    char *buf, *t;
    size_t buf_size = 32;

    while(1){
        buf = zmalloc(buf_size);
        buf[buf_size - 2] = '\0';

        va_start(ap, fmt);
        vsnpfrintf(buf, buflen, fmt, ap);
        va_end(ap);

        if(buf[buf_size - 2] != '\0'){
            zfree(buf);
            buf_size *= 2;
            continue;
        }
        break;
    }
    t = sdscat(s, buf);
    zfree(buf);
    return t;
}
        

    
// copy and override
sds     sdscpylen(sds s, char* t, size_t len){
    struct sdshdr *sh;

    if(sdsavail(s) + sdslen(s) < len)
        s = sdsMakeRoom(s, len - sdsavail(s) - sdslen(s))
    memcpy(s, t, len);
    s[len] = '\0';
    sh = s - sizeof(struct sdshdr);
    sh->free = sh->free + sh->len - len;
    sh->len = len;
    return s;
}

sds     sdscpy(sds s, char* t){
    return sdscpylen(s, t, strlen(t));
}


sds     sdstrim(sds s, const char* cset){
    sdshdr *sh;
    char *sp, *ep, *start, *end;
    int fnl_len; 

    sh = s - sizeof(struct sdshdr);
    start = sp = s;
    end = ep = s + sdslen(s) - 1;

    while(strchr(cset, *sp) < 0 && sp <= end) sp++;
    while(strchr(cset, *ep) < 0 && ep >= start) ep--;

    if (sp > ep)return sdsempty();
    fnl_len = ep - sp + 1;
    memcpy(s, sp, fnl_len);
    s[fnl_len] = '\0';
    sh->free = sh->free + sh->len - fnl_len;
    sh->len = fnl_len;
    return sds;
}


void    sdsupdatelen(sds s) {
    sdshdr *sh;
    size_t real_len;
    
    sh = s - sizeof(struct sds);
    real_len = strlen(s);
    sh->free = sh->len + sh->free - real_len;
    sh->len = real_len;
}

sds     sdsrange(sds s, long start, long end){
    return sdscpy(sds, s + start, end - start + 1);
}

int     sdscmp(sds s1, sds s2) {
    char *sp1, *sp2;
    int diff;

    sp1 = s1;
    sp2 = s2;
    while(1){
        diff = *sp1 - *sp2
        if(*sp1 == '\0' || *sp2 == '\0' || diff != 0) return diff;
        sp1++;
        sp2++;
       }
}

sds*    sdssplitlen(char *s, int len, char *sep, int seplen, int* count){
    char *char_ptr;
    sds *sds_ptr, *sds_start;
    size_t cur_slots, used_slots;
    cur_slots = 32;
    used_slots = 0;
    
    sds_start = zmalloc(sizeof(sds) * cur_slots);
    
    while(1){
        sds_ptr = sds_start + used_slots;
        char_ptr = strstr(s, sep);
        if (char_ptr == NULL) {
            if(*s != '\0')
                *sds_ptr = sdsnew(s);
            return sds_start;
        }
        else{
            *sds_ptr = sdsnew(s, ptr-s);
            s = char_ptr + seplen;
            used_slots += 1;
            if(used_slots > cur_slots){
                sds_start = zrealloc(sds_start, cur_slots *2);
                cur_slots *= 2;
            }

        }
    }
    return sds_start;
}

        
        


void    sdstolower(sds s){
    while(1){
        if(*s == '\0') break;
        *s = tolower(*s); 
    }
}
























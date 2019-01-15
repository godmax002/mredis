#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

#define DICT_HT_INITIAL_SIZE 16

typedef struct dictEntry {
    void *key;
    void *value;
    struct dictEntry *next;
} dictEntry;

typedef struct dictType {
    unsigned int (*hashFunction)(const void *key);
    void *(*keyDup)(void *privData, const void *key);
    void *(*valDup)(void *privData, const void *obj);
    int (*keyCompare)(void *privData, const void *key1, const void *key2);
    void (*keyDestructor)(void *privData, void *key);
    void (*valDestructor)(void *privData, void *obj);
} dictType;

typedef struct dict {
    dictEntry **table;
    dictType *type;
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;
    void *privData;
} dict;

typedef struct dictIterator {
    dict *ht;
    int index;
    dictEntry *entry, *nextEntry;
} dictIterator;

//macro

#define dictCompareHashKey(ht, key1, key2) \
    ((ht)->type->keyCompare ? \
     (ht)->type->keyCompare((ht)->privData, key1, key2) : \
     (key1) == (key2))

#define dictHashKey(ht, key) (ht)->type->hashFunction(key)
#define dictGetEntryKey(he) ((he)->key)
#define dictGetEntryValue(he) ((he)->value)
#define dictFreeEntryKey(ht, entry) \
    if((ht)->type->keyDestructor) \
        (ht)->type->keyDestructor((ht)->privData, entry->key)
#define dictFreeEntryVal(ht, entry) \
    if((ht)->type->valDestructor) \
        (ht)->type->valDestructor((ht)->privData, entry->value)


#define dictSetHashKey(ht, entry, _key_) do{\
    if((ht)->type->keyDup)\
        (entry)->key = (ht)->type->keyDup((ht)->privData, _key_);\
    else \
        (entry)->key = _key_;\ 
} while(0)

#define dictSetHashVal(ht, entry, _val_) do{\
    if((ht)->type->valDup)\
        (entry)->value = (ht)->type->valDup((ht)->privData, _val_);\
    else \
        (entry)->value = _val_;\
} while(0)

// api
dict *dictCreate(dictType *type, void *privDataPtr);
int dictExpand(dict *ht, unsigned int size);
int dictAdd(dict *ht, void *key, void *value);
int dictReplace(dict *ht, void *key, void *value);
int dictDelete(dict *ht, const void *key);
int dictDeleteNoFree(dict *ht, const void *key);
void dictRelease(dict *ht);
dictEntry *dictFind(dict *ht, const void *key);
int dictResize(dict *ht);
dictIterator *dictGetIterator(dict *ht);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);
dictEntry *dictGetRandomKey(dict *ht);
void dictPrintStats(dict *ht);
unsigned int dictGenHashFunction(const unsigned char *buf, int len);
void dictEmpty(dict *ht);

extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif





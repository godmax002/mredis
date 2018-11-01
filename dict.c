// api
dict *dictCreate(dictType *type, void *privDataPtr){
    dict *d;
    d = zcalloc(sizeof(struct dict));
    d->type = type;
    d->privDataPtr = privDataPtr;
    return d;
}

int dictExpand(dict *ht, unsigned int size){
    dict n;
    unsigned int realsize = _dictNextPower(size);
    if (ht->used > size)
        return DICT_ERR;
    n.type = ht->type;
    n.privdata = ht->privdata;
    n.size = realsize;
    n.sizemask = realsize - 1;
    n.table = zcallc(realsize*sizeof(dictEntry*));
    n.usde = ht->used;

    // recalculate index and move data
    for(int i=0; i<ht->size && ht->used > 0; i++){
        dictEntry *he = ht->table[i], heNext;
        if(he == NULL) continue;

        while(he){
            unsigned int keyHash;

            heNext = he->next;
            keyHash = dictHashKey(ht, he->key) & n.sizemask;
            he->next = n.table[keyHash];
            n.table[keyHash] = he;
            he = heNext;
            ht->used--;
        }
    }
    assert(ht->used == 0);
    zfree(ht->table);
    *ht = n;
    return DICT_OK;
}

int dictAdd(dict *ht, void *key, void *value){
    int index;
    dictEntry *entry;

    entry = zcalloc(sizeof(struct dictEntry));
    dictSetHashKey(ht, entry, key);
    dictSetHashValue(ht, entry, value);

    index = _dictKeyIndex(ht, key);
    entry->next = ht->table[index];
    ht->table[index] = entry;
    ht->used++;
    return DICT_OK;
}

int dictReplace(dict *ht, void *key, void *value){
    if(dictAdd(ht, key, value) == DICT_OK)
        return DICT_OK;
    dictEntry *he;
    he = dictFind(ht, key);
    dictFreeEntryVal(ht, entry);
    dictSetEntryVal(ht, entry, value);
    return DICT_OK;
}

static int dictGenericDelete(dict *ht, const void *key, int nofree){
    dictEntry he, hePrev;
    unsigned int keyHash;

    keyHash = dictHashKey(ht, key) & ht->sizemask;
    he = ht->table[keyHash];
    hePrev = NULL;

    while(he){
        if(dictCompareHashKey(ht, key, he->key)){
            if(he->prev)
                he->prev->next = he->next;
            else
                ht->table[keyHash] = he->next;
            if(!nofree){
                dictFreeEntryKey(ht, he);
                dictFreeEntrtyVal(ht, he);
            }
            zfree(he);
            ht->used--;
            return DICT_OK;
        }
        hePrev = he;
        he = he->next;
    }
    return DICT_ERR;
}
    

int dictDelete(dict *ht, const void *key){
    return dictGenericDelete(ht, key, 0);
}

int dictDeleteNoFree(dict *ht, const void *key){
    return dictGenericDelete(ht, key, 0);
}

void dictRelease(dict *ht){
    dictEntry *he, *heNext;
    while(int i=0; i<=ht->size && ht->used>0; i++){
        he = ht->table[i];
        while(he){
            heNext = he->next;
            dictFreeEntryKey(ht, he);
            dictFreeEntryVal(ht, he);
            zfree(he);
            he = heNext;
        }
        zfree(ht->table);
        zfree(ht);
    }
    return DICT_OK;
}


dictEntry *dictFind(dict *ht, const void *key){
    dictEntry he;
    unsigned int keyHash;

    keyHash = dictHashKey(ht, key) & ht->sizemask;
    he = ht->table[keyHash];

    while(he){
        if(dictCompareHashKey(ht, key, he->key))
            return he;
        he = he->next;
    }
    return NULL;
}

dictIterator *dictGetIterator(dict *ht){
    dictIterator *iter;
    iter = zcalloc(sizeof(dictIterator));
    iter->index = -1;
    return iter;
}

dictEntry *dictNext(dictIterator *iter){
    while(1){
        if(iter->entry == NULL){
            iter->index++;
            if (it->index > ht->size)
                return NULL;
            iter->entry = iter->ht->table[iter->index];
        }else{
            it->entry = it->entry->next;
        }
        if(it->entry)
            return it->entry;
    }
}

void dictReleaseIterator(dictIterator *iter){
    zfree(iter);
}

dictEntry *dictGetRandomKey(dict *ht);
void dictPrintStats(dict *ht);

unsigned int dictGenHashFunction(const unsigned char *buf, int len){
    unsigned int hash = 5381;

    // hash*33 +c
    while(len--)
        hash = ((hash<<5) + hash) + (*buf++);
    return hash;
}


void dictEmpty(dict *ht);

// private func
static int _dictExpandIfNeeded(dict *ht) {
    if(ht->size == 0)
        return dictExpand(ht, DICT_HT_INITIAL_SIZE);
    if(ht->used == ht->size)
        return dictExpand(ht, ht->size * 2);
    return DICT_OK;
}

static unsigned int _dictNextPower(unsigned int size){
    if(size >= 2147483648U)
        return 2147483648U;
    
    unsigned int i = DICT_HT_INITIAL_SIZE;
    while(i < size)
        i *= 2;
    return i;
}

static int _dictKeyIndex(dict *ht, const void *key){
    unsigned int keyHash;
    dictEntry he;

    // expand
    if(_dictExpandIfNeeded(ht) == DICT_ERR)
        return -1;
    // get key
    keyHash = dictHashKey(ht, key) & ht->sizemask;
    // test if key in dict
    he = ht->table[keyHash];
    while(he){
        if(dictCompareHashKey(ht, key, he->key))
            return -1;
        he = he->next;
    }
    return keyHash;
}



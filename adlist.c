#include "zmalloc.h"

list *listCreate(void){
    list *p;
    p = zcalloc(sizeof(struct list));
    return p;
}

list *listDup(list *orig){
    list *copy;
    listIter *iter;
    listNode *node;

    copy = listCreate();
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    
    iter = listGetIter(list, ITER_FORWARD);
    while((node=listNextElement(iter)) != NULL){
        void *node_copy;
        if(copy->dup){
            node_copy = copy->dup(node->value);
        }else{
            node_copy = node->value;
        }
        listNodeAddTail(list, node_copy);
    }
    listReleaseIter(iter);
    return copy;
}

void listRelease(list *list){
    listIter *iter;
    listNode *node;

    iter = listGetIter(list, ITER_FORWARD);
    while((node=listNextElement(iter)) != NULL) {
        listDelNode(list, node);
    }
    listReleaseIter(iter);
    zfree(list);
}


list *listNodeAddHead(list *list, void *value){
    listNode *node;
    node = zcalloc(sizeof(listNode));
    node->next = list->head;
    node->value = value;
    if(list->len == 0)
        list->tail = node;
    else
        list->head->next = node;
    list->head = node;
    list->len++;
    return list;
}


list *listNodeAddTail(list *list, void *value){
    listNode *node;
    node = zcalloc(sizeof(listNode));
    node->prev = list->tail;
    node->value = value;
    if(list->len == 0 )
        list->head = node;
    else
        list->tail->next = node;
    list->tail = node;
    list->len++;
    return list;
}

void listDelNode(list *list, listNode *node){
    if(node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if(node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    if(list->free)list->free(node->value);
    zfree(node);
    list->len--;
}

listIter *listGetIter(list *list, int direction) {
    listIter *iter;
    iter = zmalloc(sizeof(listIter));
    if(direction = ITER_FORWARD){
        iter->next = list->head;
    }else{
        iter->next = list->tail;
    }
    iter->direction = direction;
    return iter;
}

void listReleaseIter(listIter *iter){
    zfree(iter);
}

listNode *listNextElement(listIter *iter){
    void *next;
    if(iter->next){
        next = iter->next;
        iter->prev = next;
        if(iter->direction == ITER_FORWARD){
            iter->next = next->next;
        }else{
            iter->next = next->prev;
        }
            return next;
    }else
        return NULL;
}

listNode *listSearchKey(list *list, void *key){
    listIter *iter;
    listNode *node;
    iter = listGetIter(list, ITER_FORWARD);
    while((node=listNextElement(iter))!=NULL){
        if(list->match){
            if (list->match(node->value, key))
                break;
        }else{
            if (node->value == key)
                break;
        }
    }
    listReleaseIter(iter);
    return node;
}

listNode *listIndex(list *list, int index){
    listNode *node;
    while(1){
        if(index >= 0){
            node = list->head;
            if(index-- == 0)
                return node;
            else
                node = node->next;
        }else{
            node = list->tail;
            if(index++) == 0
                return node;
            else
                node = node->prev;
        }
    }
}





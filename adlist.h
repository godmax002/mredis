#ifndef ADLIST_H
#define ADLIST_H
typedef struct listNode {
    struct listNode *prev;
    struct listNode *next;
    void       *value;
} listNode;

typedef struct list {
    listNode *head;
    listNode *tail;
    int         length;
    void        (*free)(void *ptr);
    void        *(*dup)(void *ptr);
    int         (*match)(void *ptr, void *key);
} list;

typedef struct listIter {
    listNode *prev;
    listNode *next;
    int      direction;
} listIter;

#define listLength(l) ((l)->length)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)

#define listPrevNode(node) ((node)->prev)
#define listNextNode(node) ((node)->next)
#define listNodeValue(node) ((node)->value)

// method for nodes value
#define listSetFreeMethod(l, m) ((l)->free = (m))
#define listSetDupMethod(l, m) ((l)->dup = (m))
#define listSetMatchMethod(l, m) ((l)->match = (m))


#define listGetFreeMethod(l, m) ((l)->free )
#define listGetDupMethod(l, m) ((l)->dup )
#define listGetMatchMethod(l, m) ((l)->match )

list *listCreate(void);
list *listDup(list *list);
void listRelease(list *list);
list *listNodeAddHead(list *list, void *value);
list *listNodeAddTail(list *list, void *value);
void listDelNode(list *list, listNode *node);

listIter *listGetIter(list *list, int direction);
void listReleaseIter(listIter *iter);
listNode *listNextElement(listIter *iter);

listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, int index);

// iter directions
# define ITER_FORWARD 0
# define ITER_BACKWARD 1

#endif

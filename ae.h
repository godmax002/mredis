#ifndef AE_H
#define AE_H
struct aeEventLoop;

typedef void aeFileEventProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef void aeTimeEventProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop,  void *clientData);

typedef struct aeFileEvent {
    int fd;
    int mask;
    aeFileEventProc *fileProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeFileEvent *next;
} aeFileEvent;


typedef struct aeTimeEvent {
    long long id;
    long when_sec;
    long when_msec;
    aeTimeEventProc *timeProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeTimeEvent *next;
} aeTimeEvent;

typedef struct aeEventLoop {
    long long timeEventNextId;
    aeFileEvent *fileEvent;
    aeTimeEvent *timeEvent;
    int stop;
} aeEventLoop;

#define AE_OK 0
#define AE_ERR -1 

// fileEventMask
#define AE_READABLE 1
#define AE_WRITABLE 2
#define AE_EXCEPTION 4

// event
#define AE_FILEEVENT 1
#define AE_TIMEEVENT 2 
#define AE_ALLEVENT (AE_FILEEVENT|AE_TIMEEVENT)
#define AE_DONT_WAIT 4
#define AE_NO_MORE -1

#define AE_NOTUSED(V) ((void) V)
aeEventLoop *aeCreateEventLoop(void);
void *aeEventLoopStop(aeEventLoop *eventLoop);
void *aeEventLoopDelete(aeEventLoop *eventLoop);
void *aeEventLoopProcess(aeEventLoop *eventLoop, int flags);
void *aeMain(aeEventLoop *eventLoop);

void *aeWait(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop, aeFileEvent *fileEvent);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
int aeCreateTimeEvent(aeEventLoop *eventLoop, aeTimeEvent *timeEvent);
void aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);
#endif



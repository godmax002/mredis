#include <ae.h>
#include <sys/select.h>

aeEventLoop *aeEventLoopCreate(void){
    aeEventLoop *eventLoop;
    
    eventLoop = zmalloc(sizof(aeEventLoop));
    eventLoop->aeFileEvent = NULL;
    eventLoop->aeTimeEvent = NULL;
    eventLoop->timeEventNextId = 0;
    eventLoop->stop = 0;
    return eventLoop;
}

void *aeEventLoopStop(aeEventLoop *eventLoop){
    eventLoop->stop = 1;
}

void *aeEventLoopDelete(aeEventLoop *eventLoop){
    zfree(eventLoop);
}

void *aeEventLoopProcess(aeEventLoop *eventLoop, int flags){
    fd_set rfds, wfds, efds;

    aeFileEvent *fe;
    aeTimeEvent *te;
    
    fe = eventLoop->fileEvent;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    while(fe){
        if(fe->mask & AE_READABLE)
            FD_SET(fe->fd, &rfds);
        if(fe->mask & AE_WRITABLE)
            FD_SET(fe->fd, &wfds);
        if(fe->mask & AE_EXCEPTION)
            FD_SET(fe->fd, &efds);
        fe = fe->next;
    }

   



void *aeEventLoopMain(aeEventLoop *eventLoop);

void *aeWait(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop, aeFileEvent *fileEvent){
    filEvent->next = eventLoop->fileEvent;
    eventLoop->filEvent = fileEvent;
    return AE_OK;
}
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask){
    FileEvent *prev, *cur;

    prev = NULL;
    cur = eventLoop->fileEevent;

    while(cur){
        if(cur->fd == fd && cur->mask == mask){
            if(prev)
                prev->next = cur->next;
            else
                eventLoop->next = cur->next;

            if(cur->finalizerProc)
               cur->finalizerProc(eventLoop, cur->clientData); 
            zfree(cur);

            return;
        }
        prev = cur;
        cur = cur->next;
    }
}



int aeCreateTimeEvent(aeEventLoop *eventLoop, aeFileEvent *timeEvent){
    timeEvent->id = eventLoop->timeEventNextId++;

    timeEvent->next = eventLoop->timeEvent;
    eventLoop->timeEvent = timeEvent;
    return AE_OK;
}


void aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id){
    FileEvent *prev, *cur;

    prev = NULL;
    cur = eventLoop->fileEevent;

    while(cur){
        if(cur->id == id ){
            if(prev)
                prev->next = cur->next;
            else
                eventLoop->next = cur->next;

            if(cur->finalizerProc)
               cur->finalizerProc(eventLoop, cur->clientData); 
            zfree(cur);

            return;
        }
        prev = cur;
        cur = cur->next;
    }
}


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
    int maxfd = 0 ;
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
        if(maxfd < fe->fd) maxfd = fe->fd;
        fe = fe->next;
    }

    // get timeval of nearest timeEvent
    timeval tvNear, tvNow;
    aeTimeEvent *nearTE;
    nearTE = aeFindNearestTimeEvent(eventLoop);
    if(nearTE){
        gettimeofday(&tvNow);
        if(tvNow.tv_sec < nearTE->when_sec){
            if(tvNow.tv_usec < nearTE->when_msec * 1000){
                tvNear.tv_sec = nearTE->when_sec - tvNow.tv_sec;
                tvNear.tv_usec = nearTE->when_msec * 1000 - tvNow.tv_usec;
            }else{
                tvNear.tv_sec = nearTE->when_sec - tvNow.tv_sec - 1;
                tvNear.tv_usec = nearTE->when_msec * 1000 + 1000000 - tvNow.tv_usec;
            }
        }else{
            if(tvNow.tv_usec == nearTE->when_sec && tvNow.tv_usec < nearTE->when_msec * 1000){
                tvNear.tv_sec = 0;
                tvNear.tv_usec = nearTE->when_msec * 1000 - tvNow.tv_usec;
            }else{
                tvNear.tv_sec = 0;
                tvNear.tv_usec = 0;
            }
        }
    }

    // find FileEvent of ready fd, then process
    // no need to delete the fileEvent after process
    int select_ret;
    select_ret = select(maxfd + 1, &rfds, &wfds, &efds, &tvNear);
    if(select_ret){
        fe = eventLoop->fileEvent;
        while(fe){
            if((FD_ISSET(fe->fd, &rfds) && (fe->mask & AE_READABLE)) ||
                (FD_ISSET(fe->fd, &wfds) && (fe->mask & AE_WRITABLE)) ||
                (FD_ISSET(fe->fd, &efds) && (fe->mask & AE_EXCEPTION)) ){
                int mask;
                if(FD_ISSET(fe->fd, &rfds) && (fe->mask & AE_READABLE))
                    mask |= AE_READABLE;
                if(FD_ISSET(fe->fd, &wfds) && (fe->mask & AE_WRITABLE))
                    mask |= AE_WRITABLE;
                if(FD_ISSET(fe->fd, &efds) && (fe->mask & AE_EXCEPTION))
                    mask |= AE_EXCEPTION;

                fe->fileProc(eventLoop, fe->fd, fe->data, mask);
                fe = eventLoop->fileEvent;
                FD_CLR(&rfds);
                FD_CLR(&wfds);
                FD_CLR(&efds);
            }else{
                fe = fe->next;
            }
        }
    }

   // process time event, then delete them
   // 1. after every event processed, we start from begining, because some previous skipped events may be ready.
   // 2. we don't process events registerd by events processed in this loop by maxid

   int maxId = eventLoop->timeEventNextId - 1;
   te = eventLoop->timeEvent;
   while(te){
       // skip new events
       if(te->id > maxId){
           te = te->next;
       }

       gettimeofday(&tvNow);
       if(te->when_sec > tvNow.tv_sec || 
               (te->when_sec == tvNow.tv_sec && 
                te->when_msec * 1000 >= tvNow.tv_use_)){
           te->timeProc(eventLoop, te->id, te->clientData);
           aeDeleteTimeEvent(eventLoop, te->id);
           te = eventLoop->timeEvent;
       }else{
           te = te->next;
       }
   }
}


void *aeEventLoopMain(aeEventLoop *eventLoop){
    eventLoop->stop = 0;
    while(!eventLoop->stop)
        aeEventLoopProcess(eventLoop, AE_ALLEVENT);
}

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

